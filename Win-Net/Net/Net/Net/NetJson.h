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
			std::vector<void*> value;
			bool m_bSharedMemory;

		protected:
			void __push(void* ptr);

		public:
			BasicObject();
			~BasicObject();

			std::vector<void*> Value();
			void Set(std::vector<void*> value);
			void SetSharedMemory(bool m_bSharedMemory);
			bool IsSharedMemory() const;
		};

		class BasicArray
		{
		protected:
			Type m_type;
			std::vector<void*> value;
			bool m_bSharedMemory;

		protected:
			void __push(void* ptr);

		public:
			BasicArray();
			~BasicArray();

			std::vector<void*> Value();
			void Set(std::vector<void*> value);
			void SetSharedMemory(bool m_bSharedMemory);
			bool IsSharedMemory() const;
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
			void operator=(BasicObject& value);

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
			void* m_pValue;
			void* m_pParent;

		public:
			BasicValueRead(void* m_pValue, void* m_pParent);
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

			bool DeserializeAny(Net::String& key, Net::String& value, std::vector<char*>& object_chain, bool m_prepareString = false);
			bool DeserializeAny(Net::ViewString& key, Net::ViewString& value, std::vector<Net::ViewString*>& object_chain, bool m_prepareString = false);

		public:
			Object();
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
			bool Deserialize(Net::String& json, std::vector<char*>& object_chain, bool m_prepareString);
			bool Deserialize(Net::ViewString& vs, std::vector<Net::ViewString*>& object_chain, bool m_prepareString);
		};

		class Array : public BasicArray
		{
			template <typename T>
			bool emplace_back(T value, Type type);

			bool DeserializeAny(Net::String&, bool m_prepareString = false);
			bool DeserializeAny(Net::ViewString&, bool m_prepareString = false);

		public:
			Array();
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