/*
	Author: Tobias Staack
*/
#pragma once

#include <Net/Net/Net.h>

#undef NULL

template <typename T>
class vector
{
public:
	vector() : current_size(0), capacity(0), container(nullptr)
	{}

	vector(const vector<T>& copy) : current_size(0), capacity(0), container(nullptr)
	{
		*this = copy;
	}

	T* get()
	{
		return container;
	}

	T& operator[](const size_t i) const
	{
		return container[i];
	}

	void
		push_back(
			const T& value
		)
	{
		if (current_size >= capacity)
		{
			if (!capacity)
				capacity = 1;
			else
				capacity = capacity * 2;

			T* new_container = static_cast<T*>(malloc(sizeof(T) * capacity));
			if (!new_container) return;
			memset((void*)new_container, 0, sizeof(T) * capacity);

			if (current_size >= 1)
				memcpy((void*)new_container, container, current_size * sizeof(T));

			if (container != nullptr)
				free((void*)container);

			container = new_container;

			container[current_size] = value;
			current_size++;
		}
		else
		{
			container[current_size] = value;
			current_size++;
		}
	}

	void
		erase(
			const size_t erase_index
		)
	{
		if (erase_index >= 0 &&
			erase_index < current_size)
		{
			for (auto i = (erase_index + 1); i < current_size; i++)
				container[i - 1] = container[i];

			current_size--;

			if (current_size < (capacity / 2))
			{
				auto new_container = static_cast<T*>(malloc(sizeof(T) * (capacity / 2)));
				memset(new_container, 0, sizeof(T) * (capacity / 2));

				if (current_size >= 1)
					memcpy(new_container, container, current_size * sizeof(T));

				if (container != nullptr)
					free(container);

				container = new_container;
				capacity /= 2;
			}
		}
	}

	void
		insert(
			const T& value,
			const size_t index
		)
	{
		if (index >= 0 &&
			index <= current_size)
		{
			if (current_size >= capacity)
			{
				if (capacity == 0)
					capacity = 1;
				else
					capacity *= 2;

				auto new_container = static_cast<T*>(malloc(sizeof(T) * capacity));
				memset(new_container, 0, sizeof(T) * capacity);

				if (current_size >= 1)
				{
					memcpy(new_container, container, index * sizeof(T));
					new_container[index] = value;
					memcpy(&new_container[index + 1], &container[index], (current_size - index) * sizeof(T));
				}
				else
					new_container[0] = value;

				current_size++;
			}
			else
			{
				current_size++;
				for (auto i = (current_size - 1); i < index; i++)
					container[i] = container[i - 1];

				container[index] = value;
			}
		}
		else if (index >= 0 &&
			index == current_size)
			this->push_back(value);
	}

	[[nodiscard]]
	int
		size() const
	{
		return static_cast<int>(current_size);
	}

	void clear()
	{
		if (capacity > 0)
		{
			if (container)
				free(container);
		}

		current_size = 0;
		capacity = 0;
		container = nullptr;
	}

private:
	size_t current_size;
	size_t capacity;
	T* container;
	void* stack;
};

namespace Net
{
	namespace Json
	{
		class Convert
		{
		public:
			static int ToInt32(std::string& str);
			static float ToFloat(std::string& str);
			static double ToDouble(std::string& str);
			static bool ToBoolean(std::string& str);
			static bool is_float(std::string& str);
			static bool is_double(std::string& str);
			static bool is_boolean(std::string& str);
		};

		enum class Type
		{
			NULL = 0,
			OBJECT,
			ARRAY,
			STRING,
			INTEGER,
			FLOAT,
			DOUBLE,
			BOOLEAN,
		};

		enum class SerializeType
		{
			NONE = 0,
			FORMATTED
		};

		class BasicObject
		{
		protected:
			Type m_type;
			vector<void*> value;
			bool bSharedMemory;

		protected:
			void __push(void* ptr);

		public:
			BasicObject(bool bSharedMemory = false);
			~BasicObject();

			vector<void*> Value();
			void Set(vector<void*> value);
		};

		class BasicArray
		{
		protected:
			Type m_type;
			vector<void*> value;
			bool bSharedMemory;

		protected:
			void __push(void* ptr);

		public:
			BasicArray(bool bSharedMemory = false);
			~BasicArray();

			vector<void*> Value();
			void Set(vector<void*> value);
		};

		class Object;
		class Array;

		template<typename T>
		class BasicValue
		{
			Type m_type;
			char* key;
			T value;

		public:
			BasicValue();
			BasicValue(const char* key, T value, Type type);
			~BasicValue();

			void operator=(const int& value);
			void operator=(const float& value);
			void operator=(const double& value);
			void operator=(const BasicObject& value);

			void SetKey(const char* key);
			void SetValue(T value, Type type);
			void SetType(Type type);

			char* Key();
			T& Value();
			Type Type();

			bool is_null();
			bool is_object();
			bool is_array();
			bool is_integer();
			bool is_int();
			bool is_float();
			bool is_double();
			bool is_boolean();
			bool is_string();

			Object* as_object();
			Array* as_array();
			int as_int();
			float as_float();
			double as_double();
			bool as_boolean();
			char* as_string();
		};

		class NullValue : public BasicValue<void*>
		{
		public:
			NullValue();
			NullValue(int i);
		};

		class BasicValueRead
		{
			void* ptr;

		public:
			BasicValueRead(void* ptr);
			BasicValue<Object>* operator->() const;

			BasicValueRead operator[](const char* key);
			BasicValueRead operator[](char* key);
			BasicValueRead operator[](int idx);

			operator bool();

			void operator=(const NullValue& value);
			void operator=(const int& value);
			void operator=(const float& value);
			void operator=(const double& value);
			void operator=(const bool& value);
			void operator=(const char* value);
			void operator=(BasicObject& value);
			void operator=(BasicArray& value);
		};

		/* an object has no fixed data type since it stores anything json can supports */
		class Object : public BasicObject
		{
			template<typename T>
			bool __append(const char* key, T value, Type type);

			template <typename T>
			BasicValue<T>* __get(const char* key);

		public:
			Object(bool bSharedMemory = false);
			~Object();

			BasicValueRead operator[](const char* key);
			BasicValueRead At(const char* key);

			template<typename T>
			BasicValue<T>* operator=(BasicValue<T>* value);

			bool Append(const char* key, int value);
			bool Append(const char* key, float value);
			bool Append(const char* key, double value);
			bool Append(const char* key, bool value);
			bool Append(const char* key, const char* value);
			bool Append(const char* key, Object value);

			std::string Serialize(SerializeType type = SerializeType::FORMATTED, size_t iterations = 0);
			std::string Stringify(SerializeType type = SerializeType::FORMATTED, size_t iterations = 0);
			bool Deserialize(std::string json);
			bool Parse(std::string json);

			void Free();

		private:
			bool Deserialize(std::string json, vector<char*>& object_chain);
		};

		class Array : public BasicArray
		{
			template <typename T>
			bool emplace_back(T value, Type type);

		public:
			Array(bool bSharedMemory = false);
			~Array();

			BasicValueRead operator[](int idx);
			BasicValueRead at(int idx);

			bool push(int value);
			bool push(float value);
			bool push(double value);
			bool push(bool value);
			bool push(const char* value);
			bool push(Object value);
			bool push(Array value);
			bool push(Net::Json::NullValue value);

			size_t size() const;

			std::string Serialize(SerializeType type, size_t iterations = 0);
			std::string Stringify(SerializeType type, size_t iterations = 0);
			bool Deserialize(std::string json);
			bool Parse(std::string json);

			void Free();
		};

		class Document
		{
			Type m_type;
			Object root_obj;
			Array root_array;

			void Init();
			void Clear();

		public:
			Document();
			~Document();

			BasicValueRead operator[](const char* key);
			BasicValueRead operator[](int idx);
			BasicValueRead At(const char* key);
			BasicValueRead At(int idx);

			void set(Object obj);

			std::string Serialize(SerializeType type = SerializeType::FORMATTED);
			std::string Stringify(SerializeType type = SerializeType::FORMATTED);
			bool Deserialize(std::string json);
			bool Parse(std::string json);
		};
	}
}

#define null Net::Json::NullValue();