/*
	Author: Tobias Staack
*/
#pragma once
#define NET_JSON_ARR_LEN(x) sizeof(x)[0] / sizeof(x)

#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>

namespace Net
{
	namespace Json
	{
		class Document;

		template <typename T>
		class Vector
		{
		public:
			Vector() : current_size(0), capacity(0), container(nullptr), stack(nullptr)
			{
			}

			Vector(const Vector<T>& m_vector) : current_size(0), capacity(0), container(nullptr)
			{
				this->current_size = m_vector.current_size;
				this->capacity = m_vector.capacity;
		
				T* copy_container = ALLOC<T>(this->capacity);
				if (!copy_container) return;
				memcpy((void*)copy_container, m_vector.container, sizeof(T) * this->capacity);
				this->container = copy_container;

				this->stack = m_vector.stack;
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

					T* new_container = ALLOC<T>(capacity);
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
						auto new_container = ALLOC<T>(capacity / 2);
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

						auto new_container = ALLOC<T>(capacity);
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

		class Convert
		{
		public:
			static int ToInt32(char* str);
			static int ToInt32(Net::ViewString& vs);
			static float ToFloat(char* str);
			static float ToFloat(Net::ViewString& vs);
			static double ToDouble(char* str);
			static double ToDouble(Net::ViewString& vs);
			static bool ToBoolean(char* str);
			static bool ToBoolean(Net::ViewString& vs);
			static bool is_float(char* str);
			static bool is_float(Net::ViewString& vs);
			static bool is_double(char* str);
			static bool is_double(Net::ViewString& vs);
			static bool is_boolean(char* str);
			static bool is_boolean(Net::ViewString& vs);
		};

		enum class Type
		{
			NULLVALUE = 0, // fucking C and its NULL macro
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
			UNFORMATTED = 0,
			FORMATTED
		};

		class BasicObject
		{
		protected:
			Type m_type;
			Vector<void*> value;
			bool bSharedMemory;

		protected:
			void __push(void* ptr);

		public:
			BasicObject(bool bSharedMemory = false);
			~BasicObject();

			Vector<void*> Value();
			void Set(Vector<void*> value);
		};

		class BasicArray
		{
		protected:
			Type m_type;
			Vector<void*> value;
			bool bSharedMemory;

		protected:
			void __push(void* ptr);

		public:
			BasicArray(bool bSharedMemory = false);
			~BasicArray();

			Vector<void*> Value();
			void Set(Vector<void*> value);
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
			void SetKey(Net::ViewString& key);
			void SetValue(T value, Type type);
			void SetType(Type type);

			char* Key();
			T& Value();
			Type GetType();

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
			BasicValueRead operator[](Net::ViewString& key);
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
			void operator=(Document& value);
		};

		enum class EDeserializeFlag
		{
			FLAG_READING_OBJECT = (1 << 0), // reading an object
			FLAG_READING_OBJECT_VALUE = (1 << 1), // reading an element's value inside an object
			FLAG_READING_ARRAY = (1 << 2), // reading an array
			FLAG_READING_STRING = (1 << 3), // reading a string
			FLAG_READING_ELEMENT = (1 << 4), // this flag is pretty useless, just to identify in the code that we are readin an element
		};

		struct SerializeT
		{
			Net::String m_buffer;
			bool m_reserved;
		};

		/* an object has no fixed data type since it stores anything json can supports */
		class Object : public BasicObject
		{
			template<typename T>
			bool __append(const char* key, T value, Type type);

			template <typename T>
			BasicValue<T>* __get(const char* key);

			template <typename T>
			BasicValue<T>* __get(Net::ViewString& key);

			bool DeserializeAny(Net::String& key, Net::String& value, Vector<char*>& object_chain, bool m_prepareString = false);
			bool DeserializeAny(Net::ViewString& key, Net::ViewString& value, Vector<Net::ViewString*>& object_chain, bool m_prepareString = false);

		public:
			Object(bool bSharedMemory = false);
			~Object();

			BasicValueRead operator[](const char* key);
			BasicValueRead operator[](Net::ViewString& key);
			BasicValueRead At(const char* key);
			BasicValueRead At(Net::ViewString& key);

			template<typename T>
			BasicValue<T>* operator=(BasicValue<T>* value);

			bool Append(const char* key, int value);
			bool Append(const char* key, float value);
			bool Append(const char* key, double value);
			bool Append(const char* key, bool value);
			bool Append(const char* key, const char* value);
			bool Append(const char* key, Object value);

			size_t CalcLengthForSerialize();
			bool TrySerialize(SerializeType type, SerializeT& st, size_t iterations = 1);
			Net::String Serialize(SerializeType type = SerializeType::UNFORMATTED);
			Net::String Stringify(SerializeType type = SerializeType::UNFORMATTED);
			bool Deserialize(Net::String json);
			bool Deserialize(Net::ViewString& json);
			bool Deserialize(Net::String& json, bool m_prepareString);
			bool Deserialize(Net::ViewString& vs, bool m_prepareString);
			bool Parse(Net::String json);
			bool Parse(Net::ViewString& json);

			void Destroy();

		private:
			bool Deserialize(Net::String& json, Vector<char*>& object_chain, bool m_prepareString);
			bool Deserialize(Net::ViewString& vs, Vector<Net::ViewString*>& object_chain, bool m_prepareString);
		};

		class Array : public BasicArray
		{
			template <typename T>
			bool emplace_back(T value, Type type);

			bool DeserializeAny(Net::String&, bool m_prepareString = false);
			bool DeserializeAny(Net::ViewString&, bool m_prepareString = false);

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

			size_t CalcLengthForSerialize();
			bool TrySerialize(SerializeType type, SerializeT& st, size_t iterations = 1);
			Net::String Serialize(SerializeType type = SerializeType::UNFORMATTED);
			Net::String Stringify(SerializeType type = SerializeType::UNFORMATTED);
			bool Deserialize(Net::String json);
			bool Deserialize(Net::ViewString& json);
			bool Deserialize(Net::String& json, bool m_prepareString);
			bool Deserialize(Net::ViewString& json, bool m_prepareString);
			bool Parse(Net::String json);
			bool Parse(Net::ViewString& json);

			void Destroy();
		};

		class Document
		{
			Type m_type;
			Object root_obj;
			Array root_array;

			bool m_free_root_obj;
			bool m_free_root_array;

			void Init();
			void Clear();

		public:
			Document();
			~Document();

			Document& operator=(const Document& m_doc) NOEXCEPT;

			Type GetType();
			Object GetRootObject();
			Array GetRootArray();

			void SetFreeRootObject(bool);
			void SetFreeRootArray(bool);

			BasicValueRead operator[](const char* key);
			BasicValueRead operator[](int idx);
			BasicValueRead At(const char* key);
			BasicValueRead At(int idx);

			void Set(Object obj);
			void Set(Object* obj);
			void Set(Array arr);
			void Set(Array* arr);

			Net::String Serialize(SerializeType type = SerializeType::UNFORMATTED);
			Net::String Stringify(SerializeType type = SerializeType::UNFORMATTED);
			bool Deserialize(Net::String json);
			bool Deserialize(Net::ViewString& json);
			bool Parse(Net::String json);
			bool Parse(Net::ViewString& json);
		};
	}
}