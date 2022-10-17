/*
	Author: Tobias Staack
*/
#include <Net/Net/NetJson.h>
#include <Net/Cryption/XOR.h>
#include <Net/assets/manager/logmanager.h>

int Net::Json::Convert::ToInt32(Net::String& str)
{
	auto ref = str.get();
	return std::stoi(ref.get());
}

float Net::Json::Convert::ToFloat(Net::String& str)
{
	auto ref = str.get();
	return std::stof(ref.get());
}

double Net::Json::Convert::ToDouble(Net::String& str)
{
	auto ref = str.get();
	return std::stod(ref.get());
}

bool Net::Json::Convert::ToBoolean(Net::String& str)
{
	auto ref = str.get();

	if (!strcmp(CSTRING("true"), ref.get()))
		return true;
	else if (!strcmp(CSTRING("false"), ref.get()))
		return false;

	/* todo: throw error */
	return false;
}

bool Net::Json::Convert::is_float(Net::String& str)
{
	auto ref = str.get();

	char* end = nullptr;
	double val = strtof(ref.get(), &end);
	return end != str.get().get() && *end == '\0' && val != HUGE_VALF;
}

bool Net::Json::Convert::is_double(Net::String& str)
{
	auto ref = str.get();

	char* end = nullptr;
	double val = strtod(ref.get(), &end);
	return end != str.get().get() && *end == '\0' && val != HUGE_VAL;
}

bool Net::Json::Convert::is_boolean(Net::String& str)
{
	auto ref = str.get();

	if (!strcmp(CSTRING("true"), ref.get()))
		return true;
	else if (!strcmp(CSTRING("false"), ref.get()))
		return true;

	return false;
}

Net::Json::BasicObject::BasicObject(bool bSharedMemory)
{
	this->m_type = Type::OBJECT;
	this->value = {};
	this->bSharedMemory = bSharedMemory;
}

Net::Json::BasicObject::~BasicObject()
{
	this->m_type = Type::OBJECT;
	this->value = {};
	this->bSharedMemory = bSharedMemory;
}

void Net::Json::BasicObject::__push(void* ptr)
{
	value.push_back(ptr);
}

Net::Json::Vector<void*> Net::Json::BasicObject::Value()
{
	return this->value;
}

void Net::Json::BasicObject::Set(Vector<void*> value)
{
	this->value = value;
}

Net::Json::BasicArray::BasicArray(bool bSharedMemory)
{
	this->m_type = Type::ARRAY;
	this->value = {};
	this->bSharedMemory = bSharedMemory;
}

Net::Json::BasicArray::~BasicArray()
{
	this->m_type = Type::ARRAY;
	this->value = {};
	this->bSharedMemory = false;
}

void Net::Json::BasicArray::__push(void* ptr)
{
	value.push_back(ptr);
}

Net::Json::Vector<void*> Net::Json::BasicArray::Value()
{
	return this->value;
}

void Net::Json::BasicArray::Set(Vector<void*> value)
{
	this->value = value;
}

template <typename T>
Net::Json::BasicValue<T>::BasicValue()
{
	this->value = {};
	this->key = nullptr;
	this->m_type = Type::NULLVALUE;
}

template <typename T>
Net::Json::BasicValue<T>::BasicValue(const char* key, T value, Net::Json::Type type)
{
	this->SetKey(key);
	this->SetValue(value, type);
}

template <typename T>
Net::Json::BasicValue<T>::~BasicValue()
{
	/* free string from heap */
	if (this->m_type == Type::STRING)
	{
		auto cast = (BasicValue<char*>*)this;
		if (cast
			&& cast->GetType() == Type::STRING
			&& cast->Value())
		{
			delete[] cast->Value();
			cast->Value() = nullptr;
		}
	}

	this->value = {};

	if (this->key)
	{
		delete[] this->key;
		this->key = nullptr;
	}

	this->m_type = Type::NULLVALUE;
}

template <typename T>
void Net::Json::BasicValue<T>::operator=(const int& value)
{
	((BasicValue<int>*)this)->SetValue(value, Type::INTEGER);
}

template <typename T>
void Net::Json::BasicValue<T>::operator=(const float& value)
{
	((BasicValue<float>*)this)->SetValue(value, Type::FLOAT);
}

template <typename T>
void Net::Json::BasicValue<T>::operator=(const double& value)
{
	((BasicValue<double>*)this)->SetValue(value, Type::DOUBLE);
}

template <typename T>
void Net::Json::BasicValue<T>::operator=(const BasicObject& value)
{
	((BasicValue<BasicObject>*)this)->SetValue(value, Type::OBJECT);
}

template <typename T>
void Net::Json::BasicValue<T>::SetKey(const char* key)
{
	auto len = strlen(key);
	this->key = new char[len + 1];
	if (!this->key) return;
	memcpy(this->key, key, len);
	this->key[len] = 0;
}

template <typename T>
void Net::Json::BasicValue<T>::SetValue(T value, Net::Json::Type type)
{
	this->value = value;
	this->m_type = type;
}

template <typename T>
void Net::Json::BasicValue<T>::SetType(Net::Json::Type type)
{
	this->m_type = type;
}


template <typename T>
char* Net::Json::BasicValue<T>::Key()
{
	return this->key;
}

template <typename T>
T& Net::Json::BasicValue<T>::Value()
{
	return this->value;
}

template <typename T>
Net::Json::Type Net::Json::BasicValue<T>::GetType()
{
	return this->m_type;
}

template <typename T>
bool Net::Json::BasicValue<T>::is_null()
{
	return this->GetType() == Type::NULLVALUE;
}

template <typename T>
bool Net::Json::BasicValue<T>::is_object()
{
	return this->GetType() == Type::OBJECT;
}

template <typename T>
bool Net::Json::BasicValue<T>::is_array()
{
	return this->GetType() == Type::ARRAY;
}

template <typename T>
bool Net::Json::BasicValue<T>::is_integer()
{
	return this->GetType() == Type::INTEGER;
}

template <typename T>
bool Net::Json::BasicValue<T>::is_int()
{
	return this->GetType() == Type::INTEGER;
}

template <typename T>
bool Net::Json::BasicValue<T>::is_float()
{
	return this->GetType() == Type::FLOAT;
}

template <typename T>
bool Net::Json::BasicValue<T>::is_double()
{
	return this->GetType() == Type::DOUBLE;
}

template <typename T>
bool Net::Json::BasicValue<T>::is_boolean()
{
	return this->GetType() == Type::BOOLEAN;
}

template <typename T>
bool Net::Json::BasicValue<T>::is_string()
{
	return this->GetType() == Type::STRING;
}

template <typename T>
Net::Json::Object* Net::Json::BasicValue<T>::as_object()
{
	if (!is_object()) return {};
	return (Object*)&((BasicValue<BasicObject>*)this)->Value();
}

template <typename T>
Net::Json::Array* Net::Json::BasicValue<T>::as_array()
{
	if (!is_array()) return {};
	return (Array*)&((BasicValue<BasicArray>*)this)->Value();
}

template <typename T>
int Net::Json::BasicValue<T>::as_int()
{
	if (!is_integer()) return 0;
	return ((BasicValue<int>*)this)->Value();
}

template <typename T>
float Net::Json::BasicValue<T>::as_float()
{
	if (!is_float()) return 0.0f;
	return ((BasicValue<float>*)this)->Value();
}

template <typename T>
double Net::Json::BasicValue<T>::as_double()
{
	if (!is_double()) return 0;
	return ((BasicValue<double>*)this)->Value();
}

template <typename T>
bool Net::Json::BasicValue<T>::as_boolean()
{
	if (!is_boolean()) return false;
	return ((BasicValue<bool>*)this)->Value();
}

template <typename T>
char* Net::Json::BasicValue<T>::as_string()
{
	if (!is_string()) return (char*)"";
	return ((BasicValue<char*>*)this)->Value();
}

Net::Json::NullValue::NullValue()
{
	this->SetType(Type::NULLVALUE);
}

Net::Json::NullValue::NullValue(int i)
{
	this->SetType(Type::NULLVALUE);
}

Net::Json::BasicValueRead::BasicValueRead(void* ptr)
{
	this->ptr = ptr;
}

Net::Json::BasicValue<Net::Json::Object>* Net::Json::BasicValueRead::operator->() const
{
	return (BasicValue<Object>*)this->ptr;
}

static Net::Json::BasicValueRead object_to_BasicValueRead(void* ptr, const char* key)
{
	if (!ptr) return { nullptr };
	auto cast = (Net::Json::BasicValueRead*)ptr;
	if (!cast) return { nullptr };
	auto cast2 = (Net::Json::BasicValue<Net::Json::Object>*)cast->operator->();
	if (!cast2) return { nullptr };
	if (cast2->GetType() != Net::Json::Type::OBJECT) return { nullptr };
	return cast2->Value()[key];
}

static Net::Json::BasicValueRead object_to_BasicValueRead(void* ptr, int idx)
{
	if (!ptr) return { nullptr };
	auto cast = (Net::Json::BasicValueRead*)ptr;
	if (!cast) return { nullptr };
	auto cast2 = (Net::Json::BasicValue<Net::Json::Array>*)cast->operator->();
	if (!cast2) return { nullptr };
	if (cast2->GetType() != Net::Json::Type::ARRAY) return { nullptr };
	return cast2->Value()[idx];
}

Net::Json::BasicValueRead Net::Json::BasicValueRead::operator[](const char* key)
{
	return object_to_BasicValueRead(this, key);
}

Net::Json::BasicValueRead Net::Json::BasicValueRead::operator[](char* key)
{
	return object_to_BasicValueRead(this, key);
}

Net::Json::BasicValueRead Net::Json::BasicValueRead::operator[](int idx)
{
	return object_to_BasicValueRead(this, idx);
}

Net::Json::BasicValueRead::operator bool()
{
	if (!this->ptr) return false;

	auto cast = (Net::Json::BasicValueRead*)ptr;
	if (!cast) return false;

	return true;
}

void Net::Json::BasicValueRead::operator=(const NullValue& value)
{
	if (!this->ptr) return;
	((BasicValue<NullValue>*)this->ptr)->SetValue(value, Type::NULLVALUE);
}

void Net::Json::BasicValueRead::operator=(const int& value)
{
	if (!this->ptr) return;
	((BasicValue<int>*)this->ptr)->SetValue(value, Type::INTEGER);
}

void Net::Json::BasicValueRead::operator=(const float& value)
{
	if (!this->ptr) return;
	((BasicValue<float>*)this->ptr)->SetValue(value, Type::FLOAT);
}

void Net::Json::BasicValueRead::operator=(const double& value)
{
	if (!this->ptr) return;
	((BasicValue<double>*)this->ptr)->SetValue(value, Type::DOUBLE);
}

void Net::Json::BasicValueRead::operator=(const bool& value)
{
	if (!this->ptr) return;
	((BasicValue<bool>*)this->ptr)->SetValue(value, Type::BOOLEAN);
}

void Net::Json::BasicValueRead::operator=(const char* value)
{
	if (!this->ptr) return;

	size_t len = strlen(value);
	char* ptr = new char[len + 1];
	memcpy(ptr, value, len);
	ptr[len] = 0;

	auto cast = ((BasicValue<char*>*)this->ptr);
	if (cast->GetType() == Type::STRING
		&& cast->Value())
	{
		delete[] cast->Value();
		cast->Value() = nullptr;
	}
	cast->SetValue(ptr, Type::STRING);
}

void Net::Json::BasicValueRead::operator=(BasicObject& value)
{
	if (!this->ptr) return;
	((BasicValue<BasicObject>*)this->ptr)->SetValue(value, Type::OBJECT);
}

void Net::Json::BasicValueRead::operator=(BasicArray& value)
{
	if (!this->ptr) return;
	((BasicValue<BasicArray>*)this->ptr)->SetValue(value, Type::ARRAY);
}

void Net::Json::BasicValueRead::operator=(Document& value)
{
	if (!this->ptr) return;

	// copy the document
	switch (value.GetType())
	{
	case Net::Json::Type::OBJECT:
		((BasicValue<BasicObject>*)this->ptr)->SetValue(value.GetRootObject(), Type::OBJECT);
		value.SetFreeRootObject(false);
		break;

	case Net::Json::Type::ARRAY:
		((BasicValue<BasicArray>*)this->ptr)->SetValue(value.GetRootArray(), Type::ARRAY);
		value.SetFreeRootArray(false);
		break;

	default:
		// error
		NET_LOG_ERROR(CSTRING("[Json] - Unable to copy document => invalid type"));
		break;
	}
}

Net::Json::Object::Object(bool bSharedMemory)
	: Net::Json::BasicObject::BasicObject(bSharedMemory)
{
}

Net::Json::Object::~Object()
{
	if (!bSharedMemory) this->Free();
}

void Net::Json::Object::Free()
{
	this->value.clear();
}

template<typename T>
bool Net::Json::Object::__append(const char* key, T value, Type type)
{
	BasicValue<T>* heap = new BasicValue<T>();
	if (!heap) return false;
	heap->SetKey(key);
	heap->SetValue(value, type);
	this->__push(heap);
	return true;
}

template <typename T>
Net::Json::BasicValue<T>* Net::Json::Object::__get(const char* key)
{
	for (size_t i = 0; i < value.size(); ++i)
	{
		BasicValue<T>* tmp = (BasicValue<T>*)value[i];
		if (strcmp(tmp->Key(), key) != 0) continue;
		return (BasicValue<T>*)value[i];
	}

	return nullptr;
}

Net::Json::BasicValueRead Net::Json::Object::operator[](const char* key)
{
	return this->At(key);
}

Net::Json::BasicValueRead Net::Json::Object::At(const char* key)
{
	auto ptr = this->__get<Object>(key);
	if (!ptr)
	{
		BasicValue<Object>* heap = new BasicValue<Object>();
		if (!heap) return { nullptr };
		heap->SetType(Type::OBJECT);
		heap->SetKey(key);
		this->__push(heap);
		return { heap };
	}

	return { ptr };
}

template<typename T>
Net::Json::BasicValue<T>* Net::Json::Object::operator=(BasicValue<T>* value)
{
	this->__push(value);
	return value;
}

bool Net::Json::Object::Append(const char* key, int value)
{
	return __append(key, value, Type::INTEGER);
}

bool Net::Json::Object::Append(const char* key, float value)
{
	return __append(key, value, Type::FLOAT);
}

bool Net::Json::Object::Append(const char* key, double value)
{
	return __append(key, value, Type::DOUBLE);
}

bool Net::Json::Object::Append(const char* key, bool value)
{
	return __append(key, value, Type::BOOLEAN);
}

bool Net::Json::Object::Append(const char* key, const char* value)
{
	size_t len = strlen(value);
	char* ptr = new char[len + 1];
	memcpy(ptr, value, len);
	ptr[len] = 0;
	if (!__append(key, ptr, Type::STRING))
	{
		delete[] ptr;
		ptr = nullptr;
		return false;
	}

	return true;
}

bool Net::Json::Object::Append(const char* key, Object value)
{
	return __append(key, value, Type::OBJECT);
}

Net::String Net::Json::Object::Serialize(SerializeType type, size_t iterations)
{
	Net::String out = CSTRING("");

	out += (type == SerializeType::FORMATTED ? CSTRING("{\n") : CSTRING("{"));

	for (size_t i = 0; i < value.size(); ++i)
	{
		auto tmp = (BasicValue<void*>*)value[i];
		if (tmp->GetType() == Type::NULLVALUE)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out.append(CSTRING(R"("%s":%s)"), tmp->Key(), CSTRING("null"));
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::STRING)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			Net::String str(tmp->as_string());
			str.replaceAll(CSTRING("\n"), CSTRING("\\n"));

			out.append(CSTRING(R"("%s":"%s")"), tmp->Key(), str.get().get());
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::INTEGER)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out.append(CSTRING(R"("%s":%i)"), tmp->Key(), tmp->as_int());
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::FLOAT)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out.append(CSTRING(R"("%s":%F)"), tmp->Key(), tmp->as_float());
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::DOUBLE)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out.append(CSTRING(R"("%s":%F)"), tmp->Key(), tmp->as_double());
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::BOOLEAN)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out.append(CSTRING(R"("%s":%s)"), tmp->Key(), tmp->as_boolean() ? CSTRING("true") : CSTRING("false"));
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::OBJECT)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out.append(CSTRING(R"("%s":)"), tmp->Key());
			out += tmp->as_object()->Serialize(type, (iterations + 1));
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::ARRAY)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out.append(CSTRING(R"("%s":)"), tmp->Key());
			out += tmp->as_array()->Serialize(type, (iterations + 1));
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
	}

	size_t it = 0;
	if ((it = out.findLastOf(CSTRING(","))) != NET_STRING_NOT_FOUND)
		out.erase(it, 1);

	if (type == SerializeType::FORMATTED) out += CSTRING("\n");

	if (type == SerializeType::FORMATTED)
	{
		for (size_t it = 0; it < iterations; ++it)
		{
			out += CSTRING("\t");
		}
	}

	out += CSTRING("}");

	return out;
}

Net::String Net::Json::Object::Stringify(SerializeType type, size_t iterations)
{
	return this->Serialize(type, iterations);
}

/* wrapper */
bool Net::Json::Object::Deserialize(Net::String json, bool m_removedWhitespace)
{
	Vector<char*> object_chain = {};
	auto ret = this->Deserialize(json, object_chain, m_removedWhitespace);
	for (size_t i = 0; i < object_chain.size(); ++i)
	{
		if (!object_chain[i]) continue;
		delete[] object_chain[i];
		object_chain[i] = nullptr;
	}
	return ret;
}

bool Net::Json::Object::Parse(Net::String json)
{
	return this->Deserialize(json);
}

/*
* this method will validate the value
* also, it will push it to the memory
* it uses recursive calls to deserialize children from object's and array's
*/
bool Net::Json::Object::DeserializeAny(Net::String& key, Net::String& value, Vector<char*>& object_chain, bool m_removedWhitespace)
{
	Net::Json::Type m_type = Net::Json::Type::NULLVALUE;
	auto refValue = value.get();

	// with object chain
	Net::Json::BasicValueRead obj(nullptr);
	if (object_chain.size() > 0)
	{
		obj = { this->operator[](object_chain[0]) };
		for (size_t i = 1; i < object_chain.size(); ++i)
		{
			obj = obj[object_chain[i]];
		}
	}

	/*
	* check for object
	*/
	{
		if (refValue.get()[0] == '{')
		{
			m_type = Net::Json::Type::OBJECT;
		}

		if (refValue.get()[value.length() - 1] == '}'
			&& m_type != Net::Json::Type::OBJECT)
		{
			// we got an ending curly for an object, but missing the starting curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad object ... got an ending curly for an object, but missing the starting curly ... '%s'"), refValue.get());
			return false;
		}
		else if (refValue.get()[value.length() - 1] != '}'
			&& m_type == Net::Json::Type::OBJECT)
		{
			// we got a starting curly for an object, but missing the ending curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad object ... got a starting curly for an object, but missing the ending curly ... '%s'"), refValue.get());
			return false;
		}
		else if (m_type == Net::Json::Type::OBJECT)
		{
			// object seem to be fine, now call it's deserializer

#ifdef BUILD_LINUX
			object_chain.push_back(strdup(key.get().get()));
#else
			object_chain.push_back(_strdup(key.get().get()));
#endif

			// need this line otherwise empty objects do not work
			if (object_chain.size() > 0)
			{
				obj[key.get().get()] = Net::Json::Object();
			}
			else
			{
				this->operator[](key.get().get()) = Net::Json::Object();
			}

			if (!this->Deserialize(value, object_chain, m_removedWhitespace))
				return false;

			object_chain.erase(object_chain.size() - 1);

			return true;
		}
	}

	/*
	* check for array
	*/
	{
		if (refValue.get()[0] == '[')
		{
			m_type = Net::Json::Type::ARRAY;
		}

		if (refValue.get()[value.length() - 1] == ']'
			&& m_type != Net::Json::Type::ARRAY)
		{
			// we got an ending curly for an array, but missing the starting curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad array ... got an ending curly for an array, but missing the starting curly ... '%s'"), refValue.get());
			return false;
		}
		else if (refValue.get()[value.length() - 1] != ']'
			&& m_type == Net::Json::Type::ARRAY)
		{
			// we got a starting curly for an array, but missing the ending curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad array ... got a starting curly for an array, but missing the ending curly ... '%s'"), refValue.get());
			return false;
		}
		else if (m_type == Net::Json::Type::ARRAY)
		{
			// array seem to be fine, now call it's deserializer

			Net::Json::Array arr(true);
			if (!arr.Deserialize(value, m_removedWhitespace))
			{
				// @todo: add logging
				return false;
			}

			if (object_chain.size() > 0)
			{
				obj[key.get().get()] = arr;
			}
			else
			{
				this->operator[](key.get().get()) = arr;
			}

			return true;
		}
	}

	/*
	* check for string
	*/
	{
		if (refValue.get()[0] == '"')
		{
			m_type = Net::Json::Type::STRING;
		}

		if (refValue.get()[value.length() - 1] == '"'
			&& m_type != Net::Json::Type::STRING)
		{
			// we got an ending double-qoute for a string, but missing the starting
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad string ... got an ending double-qoute for a string, but missing the starting ... '%s'"), refValue.get());
			return false;
		}
		else if (refValue.get()[value.length() - 1] != '"'
			&& m_type == Net::Json::Type::STRING)
		{
			// we got a starting double-qoute for a string, but missing the ending
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad string ... got a starting double-qoute for a string, but missing the ending ... '%s'"), refValue.get());
			return false;
		}
		else if (m_type == Net::Json::Type::STRING)
		{
			/*
			* walk through the string and make sure there are no non-escaped double-qoutes
			*/
			for (int j = 1; j < value.length() - 1; ++j)
			{
				auto ec = refValue.get()[j];
				if (ec == '"')
				{
					if ((j - 1) < 0
						|| refValue.get()[j - 1] != '\\')
					{
						NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad string ... string contains double-qoutes that are not escaped ... double-qoutes inside a string must be escaped with '\\'"));
						return false;
					}
				}
			}

			// obtain the string from the json-string without the double-qoutes
			auto str = value.substr(1, value.length() - 2);

			if (object_chain.size() > 0)
			{
				obj[key.get().get()] = str;
			}
			else
			{
				this->operator[](key.get().get()) = str;
			}

			return true;
		}
	}

	/*
	* check for boolean
	*/
	{
		if (Convert::is_boolean(value))
		{
			m_type = Net::Json::Type::BOOLEAN;

			if (object_chain.size() > 0)
			{
				obj[key.get().get()] = Convert::ToBoolean(value);
				return true;
			}

			this->operator[](key.get().get()) = Convert::ToBoolean(value);
			return true;
		}
	}

	/*
	* check for null value
	*/
	{
		if (!memcmp(refValue.get(), CSTRING("null"), 4))
		{
			m_type = Net::Json::Type::NULLVALUE;

			if (object_chain.size() > 0)
			{
				obj[key.get().get()] = Net::Json::NullValue();
				return true;
			}

			this->operator[](key.get().get()) = Net::Json::NullValue();
			return true;
		}
	}

	/*
	* check for number
	*/
	{
		m_type = Net::Json::Type::INTEGER;

		constexpr const char m_NumericPattern[] = "0123456789";
		for (size_t i = 0, dot = 0; i < value.size(); ++i)
		{
			auto c = refValue.get()[i];

			/*
			* check for decimal number
			*/
			if (c == '.')
			{
				if (dot > 0)
				{
					// number got multiplie decimal splitter
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad number ... number has more than one of the decimal ('.') splitter"));
					return false;
				}

				/*
				* decimal number
				*/
				++dot;

				m_type = Net::Json::Type::DOUBLE;
				continue;
			}

			bool m_isNumber = false;
			for (size_t z = 0; z < sizeof(m_NumericPattern); ++z)
			{
				if (static_cast<int>(c) == static_cast<int>(m_NumericPattern[z]))
				{
					m_isNumber = true;
				}
			}

			if (!m_isNumber)
			{
				/* not a number */

				/*
				* null value is handled above, so if the code is reaching the end and the type is still null
				* then the value is not valid format
				*/
				m_type = Net::Json::Type::NULLVALUE;
				break;
			}
		}

		/* value is definitely a number */
		if (m_type != Net::Json::Type::NULLVALUE)
		{
			/*
			* value is a decimal number
			*/
			if (m_type == Net::Json::Type::DOUBLE)
			{
				/*
				* determinate if it is worth a double or just a float
				*/

				if (Convert::is_double(value))
				{
					m_type = Net::Json::Type::DOUBLE;
				}
				else if (Convert::is_float(value))
				{
					m_type = Net::Json::Type::FLOAT;
				}
			}

			switch (m_type)
			{
			case Net::Json::Type::INTEGER:
				if (object_chain.size() > 0)
				{
					obj[key.get().get()] = Convert::ToInt32(value);
					break;
				}

				this->operator[](key.get().get()) = Convert::ToInt32(value);
				break;

			case Net::Json::Type::DOUBLE:
				if (object_chain.size() > 0)
				{
					obj[key.get().get()] = Convert::ToDouble(value);
					break;
				}

				this->operator[](key.get().get()) = Convert::ToDouble(value);
				break;

			case Net::Json::Type::FLOAT:
				if (object_chain.size() > 0)
				{
					obj[key.get().get()] = Convert::ToFloat(value);
					break;
				}

				this->operator[](key.get().get()) = Convert::ToFloat(value);
				break;
			}

			return true;
		}
	}

	NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad value ... value is from unknown type ... got '%s'"), refValue.get());
	return false;
}

/*
* @Todo: Add better parsing error detection
* How it is working in its current state
*	- reading till the common seperator ':' reached
*		- then extracting the key & value from each other
*	- processing the value
*		- using recursive method to parse object chain
*		- creating an array object on parsing array
*
*	Rework:
*		- What does it do so far:
*			- it checks if the json string starts of with '{' and ends with '}'
*			- it reads the single elements inside of the object and checks for its split that determinate the key and the value
*			- it reads the entire object or array and stores it as value (use recursive for further analyses)
*			- it realises if any splitters are missing or if too many have been placed
*				- it does not do this very accurate but good enough
*			- it checks if the key is a string
*			- it checks if the key contains any double-qoutes and if it does then it checks if they are escaped
*/
bool Net::Json::Object::Deserialize(Net::String& json, Vector<char*>& object_chain, bool m_removedWhitespace)
{
	/*
	* remove whitespace and line breaks
	*/
	if (!m_removedWhitespace)
	{
		json.eraseAll(' ');
		json.eraseAll('\n');
		m_removedWhitespace = !m_removedWhitespace;
	}

	/*
	* Since we are passing the json string into an object deserializer
	* we will check for the syntax that does define an object in the json language at the beginning and end
	* if not then abort the parsing
	*/
	if (json.get().get()[0] != '{')
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Unexpected character in the beginning of the string ... got '%c' ... expected '{'"), json.get().get()[0]);
		this->Free();
		return false;
	}

	if (json.get().get()[json.length() - 1] != '}')
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Unexpected character in the ending of the string ... got '%c' ... expected '}'"), json.get().get()[json.length() - 1]);
		this->Free();
		return false;
	}

	uint8_t flag = 0;
	size_t v = 0; // begin for substr
	size_t arr_count = 0;
	size_t obj_count = 0;
	auto ref = json.get();

	Net::String key(CSTRING(""));

	/*
	* So an object is seperated in its key and value pair
	* first scan for the key and detect its syntax that deals as the seperator (':')
	*/
	flag |= (int)EDeserializeFlag::FLAG_READING_ELEMENT;
	v = 0;

	/*
	* Since we did check the start and end to match its syntax that does define an object
	* so we can skip the first and last character for the further parsing
	*/
	for (size_t i = 1; i < json.size() - 2; ++i)
	{
		auto c = ref.get()[i];

		if (flag & (int)EDeserializeFlag::FLAG_READING_ELEMENT)
		{
			/*
			* Spotted beginning of an object
			* now read it fully before further analysing
			*/
			if (c == '{')
			{
				++obj_count;
				flag |= (int)EDeserializeFlag::FLAG_READING_OBJECT;
			}
			else if (c == '}')
			{
				if (!(flag & (int)EDeserializeFlag::FLAG_READING_OBJECT)
					|| obj_count == 0)
				{
					// we got an ending curly for an object but missing the start curly
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad syntax ... expected '{' ... got '}'"));
					this->Free();
					return false;
				}

				--obj_count;

				if (obj_count == 0)
				{
					/*
					* ok, finished reading the object
					*/
					flag &= ~(int)EDeserializeFlag::FLAG_READING_OBJECT;
				}
			}

			/*
			* Spotted beginning of an array
			* now read it fully before further analysing
			*/
			if (c == '[')
			{
				++arr_count;
				flag |= (int)EDeserializeFlag::FLAG_READING_ARRAY;
			}
			else if (c == ']')
			{
				if (!(flag & (int)EDeserializeFlag::FLAG_READING_ARRAY)
					|| arr_count == 0)
				{
					// we got an ending curly for an arry but missing the start curly
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad syntax ... expected '[' ... got ']'"));
					this->Free();
					return false;
				}

				--arr_count;

				if (arr_count == 0)
				{
					/*
					* ok, finished reading the array
					*/
					flag &= ~(int)EDeserializeFlag::FLAG_READING_ARRAY;
				}
			}

			/*
			* Spotted beginning of a string
			* now read it fully
			*/
			if (c == '"'
				&& !(flag & (int)EDeserializeFlag::FLAG_READING_OBJECT
					|| flag & (int)EDeserializeFlag::FLAG_READING_ARRAY))
			{
				if (flag & (int)EDeserializeFlag::FLAG_READING_STRING)
				{
					flag &= ~(int)EDeserializeFlag::FLAG_READING_STRING;
				}
				else
				{
					flag |= (int)EDeserializeFlag::FLAG_READING_STRING;
				}
			}

			/*
			* Seperator spotted
			* now walk back to determinate the key
			* and walk forward to determinate the value
			* to determinate the value walk till we reach the syntax that defines the seperation between elements inside an object
			* or simply till we reach the end, this will be the case if there is only one element
			*/
			if (!(flag & (int)EDeserializeFlag::FLAG_READING_OBJECT)
				&& !(flag & (int)EDeserializeFlag::FLAG_READING_ARRAY)
				&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING)
				&& c == ':')
			{
				if (flag & (int)EDeserializeFlag::FLAG_READING_OBJECT_VALUE)
				{
					// we got a seperator for key and value pair but were reading the value?
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad syntax ... expected to read the value of the element ... got another ':' instead"));
					this->Free();
					return false;
				}

				/*
				* the last key is obviously the last position at which we set the flag to reading the key
				* now we have determinated the key, but the value is not determinated yet
				* walk forward till we reach the syntax for the seperator for an element or till we reach the end of file
				*/
				key = json.substr(v + 1, i - v - 1);
				auto refKey = key.get();

				/*
				* a key in the json language must be a string
				* and should be unique in each element
				*/
				if (refKey.get()[0] != '"')
				{
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad key ... key is not a string ... it must start with double quotes ... instead got '%c'"), key.get().get()[0]);
					this->Free();
					return false;
				}

				if (refKey.get()[key.length() - 1] != '"')
				{
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad key ... key is not a string ... it must end with double quotes ... instead got '%c'"), key.get().get()[key.length() - 1]);
					this->Free();
					return false;
				}

				/*
				* walk through the key and make sure there are no non-escaped double-qoutes
				*/
				for (int j = 1; j < key.length() - 1; ++j)
				{
					auto ec = refKey.get()[j];
					if (ec == '"')
					{
						if ((j - 1) < 0
							|| refKey.get()[j - 1] != '\\')
						{
							NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad key ... key contains double-qoutes that are not escaped ... double-qoutes inside a key must be escaped with '\\'"));
							this->Free();
							return false;
						}
					}
				}

				// obtain the string from the json-string without the double-qoutes
				key = key.substr(1, key.length() - 2);

				flag |= (int)EDeserializeFlag::FLAG_READING_OBJECT_VALUE;
				v = i;
			}

			/*
			* check for the syntax that seperates elements inside an object
			* or check if we have reached eof (end of file)
			*/
			if (c == ',' || (i == json.size() - 3))
			{
				/*
				* keep going until we read the entire object or array
				* only stop if we reached eof
				*/
				if (c == ','
					&& (flag & (int)EDeserializeFlag::FLAG_READING_OBJECT 
						|| flag & (int)EDeserializeFlag::FLAG_READING_ARRAY
						|| flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					continue;
				}

				/*
				* Spotted seperator for an element
				* now determinate the value
				*/
				if (!(flag & (int)EDeserializeFlag::FLAG_READING_OBJECT_VALUE))
				{
					// we got a seperator for an element but missing the splitter for the key and value pair
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad syntax ... expected to read the elements key ... instead got ',' OR 'EOF'"));
					this->Free();
					return false;
				}

				if ((flag & (int)EDeserializeFlag::FLAG_READING_OBJECT) && obj_count > 0)
				{
					// we got another seperator for an element but missing the end curly for an object
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad syntax ... expected '}' ... got ',' OR 'EOF'"));
					this->Free();
					return false;
				}

				if ((flag & (int)EDeserializeFlag::FLAG_READING_ARRAY) && arr_count > 0)
				{
					// we got another seperator for an element but missing the end curly for an array
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad syntax ... expected ']' ... got ',' OR 'EOF'"));
					this->Free();
					return false;
				}

				Net::String value = (i == json.size() - 3) ? json.substr(v + 1, i - v) : json.substr(v + 1, i - v - 1);

				/*
				* pass the value to the DeserializeAny method
				* that method will take any value and further analyse its type
				* it will use recursive to deserialize further object's or array's
				*/
				if (!DeserializeAny(key, value, object_chain, m_removedWhitespace))
				{
					// no need to display error message, the method will do it
					this->Free();
					return false;
				}

				/*
				* value determinated
				* now remove the flag
				*/
				flag &= ~(int)EDeserializeFlag::FLAG_READING_OBJECT_VALUE;
				v = i;
			}
		}
	}

	if (arr_count > 0)
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad syntax ... expected an array ending curly ']'"));
		this->Free();
		return false;
	}

	if (obj_count > 0)
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad syntax ... expected an object ending curly '}'"));
		this->Free();
		return false;
	}

	return true;
}

Net::Json::Array::Array(bool bSharedMemory)
	: Net::Json::BasicArray::BasicArray(bSharedMemory)
{
}

Net::Json::Array::~Array()
{
	if (!bSharedMemory) this->Free();
}

void Net::Json::Array::Free()
{
	this->value.clear();
}

template <typename T>
bool Net::Json::Array::emplace_back(T value, Type type)
{
	BasicValue<T>* heap = new BasicValue<T>();
	if (!heap) return false;
	heap->SetValue(value, type);
	this->__push(heap);
	return true;
}

Net::Json::BasicValueRead Net::Json::Array::operator[](int idx)
{
	if (idx >= value.size()) return { nullptr };
	return { this->value[idx] };
}

Net::Json::BasicValueRead Net::Json::Array::at(int idx)
{
	return this->operator[](idx);
}

bool Net::Json::Array::push(int value)
{
	return this->emplace_back(value, Type::INTEGER);
}

bool Net::Json::Array::push(float value)
{
	return this->emplace_back(value, Type::FLOAT);
}

bool Net::Json::Array::push(double value)
{
	return this->emplace_back(value, Type::DOUBLE);
}

bool Net::Json::Array::push(bool value)
{
	return this->emplace_back(value, Type::BOOLEAN);
}

bool Net::Json::Array::push(const char* value)
{
	size_t len = strlen(value);
	char* ptr = new char[len + 1];
	memcpy(ptr, value, len);
	ptr[len] = 0;

	return this->emplace_back(ptr, Type::STRING);
}

bool Net::Json::Array::push(Object value)
{
	return this->emplace_back(value, Type::OBJECT);
}

bool Net::Json::Array::push(Array value)
{
	return this->emplace_back(value, Type::ARRAY);
}

bool Net::Json::Array::push(Net::Json::NullValue value)
{
	return this->emplace_back(value, Type::NULLVALUE);
}

size_t Net::Json::Array::size() const
{
	return this->value.size();
}

Net::String Net::Json::Array::Serialize(SerializeType type, size_t iterations)
{
	Net::String out = CSTRING("");

	out += (type == SerializeType::FORMATTED ? CSTRING("[\n") : CSTRING("["));

	for (size_t i = 0; i < value.size(); ++i)
	{
		auto tmp = (BasicValue<void*>*)value[i];
		if (tmp->GetType() == Type::NULLVALUE)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out += Net::String(CSTRING(R"(%s)"), CSTRING("null"));
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::STRING)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			Net::String str(tmp->as_string());
			str.replaceAll(CSTRING("\n"), CSTRING("\\n"));

			out += Net::String(CSTRING(R"("%s")"), str.get().get());
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::INTEGER)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out += Net::String(CSTRING(R"(%i)"), tmp->as_int());
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::FLOAT)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out += Net::String(CSTRING(R"(%F)"), tmp->as_float());
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::DOUBLE)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out += Net::String(CSTRING(R"(%F)"), tmp->as_double());
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::BOOLEAN)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out += Net::String(CSTRING(R"(%s)"), tmp->as_boolean() ? CSTRING("true") : CSTRING("false"));
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::OBJECT)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out += tmp->as_object()->Serialize(type, (iterations + 1));
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
		else if (tmp->GetType() == Type::ARRAY)
		{
			if (type == SerializeType::FORMATTED)
			{
				for (size_t it = 0; it < iterations + 1; ++it)
				{
					out += CSTRING("\t");
				}
			}

			out += tmp->as_array()->Serialize(type, (iterations + 1));
			out += (type == SerializeType::FORMATTED) ? CSTRING(",\n") : CSTRING(",");
		}
	}

	size_t it = 0;
	if ((it = out.findLastOf(CSTRING(","))) != NET_STRING_NOT_FOUND)
		out.erase(it, 1);

	if (type == SerializeType::FORMATTED) out += CSTRING("\n");

	if (type == SerializeType::FORMATTED)
	{
		for (size_t it = 0; it < iterations; ++it)
		{
			out += CSTRING("\t");
		}
	}

	out += CSTRING("]");
	return out;
}

Net::String Net::Json::Array::Stringify(SerializeType type, size_t iterations)
{
	return this->Serialize(type, iterations);
}

bool Net::Json::Array::DeserializeAny(Net::String& value, bool m_removedWhitespace)
{
	Net::Json::Type m_type = Net::Json::Type::NULLVALUE;
	auto refValue = value.get();

	/*
	* check for object
	*/
	{
		if (refValue.get()[0] == '{')
		{
			m_type = Net::Json::Type::OBJECT;
		}

		if (refValue.get()[value.length() - 1] == '}'
			&& m_type != Net::Json::Type::OBJECT)
		{
			// we got an ending curly for an object, but missing the starting curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad object ... got an ending curly for an object, but missing the starting curly ... '%s'"), refValue.get());
			return false;
		}
		else if (refValue.get()[value.length() - 1] != '}'
			&& m_type == Net::Json::Type::OBJECT)
		{
			// we got a starting curly for an object, but missing the ending curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad object ... got a starting curly for an object, but missing the ending curly ... '%s'"), refValue.get());
			return false;
		}
		else if (m_type == Net::Json::Type::OBJECT)
		{
			// object seem to be fine, now call it's deserializer

			Net::Json::Object obj(true);
			if (!obj.Deserialize(value, m_removedWhitespace))
			{
				// @todo: add logging
				return false;
			}
			this->push(obj);

			return true;
		}
	}

	/*
	* check for array
	*/
	{
		if (refValue.get()[0] == '[')
		{
			m_type = Net::Json::Type::ARRAY;
		}

		if (refValue.get()[value.length() - 1] == ']'
			&& m_type != Net::Json::Type::ARRAY)
		{
			// we got an ending curly for an array, but missing the starting curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad array ... got an ending curly for an array, but missing the starting curly ... '%s'"), refValue.get());
			return false;
		}
		else if (refValue.get()[value.length() - 1] != ']'
			&& m_type == Net::Json::Type::ARRAY)
		{
			// we got a starting curly for an array, but missing the ending curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad array ... got a starting curly for an array, but missing the ending curly ... '%s'"), refValue.get());
			return false;
		}
		else if (m_type == Net::Json::Type::ARRAY)
		{
			// array seem to be fine, now call it's deserializer

			Net::Json::Array arr(true);
			if (!arr.Deserialize(value, m_removedWhitespace))
			{
				// @todo: add logging
				return false;
			}
			this->push(arr);

			return true;
		}
	}

	/*
	* check for string
	*/
	{
		if (refValue.get()[0] == '"')
		{
			m_type = Net::Json::Type::STRING;
		}

		if (refValue.get()[value.length() - 1] == '"'
			&& m_type != Net::Json::Type::STRING)
		{
			// we got an ending double-qoute for a string, but missing the starting
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad string ... got an ending double-qoute for a string, but missing the starting ... '%s'"), refValue.get());
			return false;
		}
		else if (refValue.get()[value.length() - 1] != '"'
			&& m_type == Net::Json::Type::STRING)
		{
			// we got a starting double-qoute for a string, but missing the ending
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad string ... got a starting double-qoute for a string, but missing the ending ... '%s'"), refValue.get());
			return false;
		}
		else if (m_type == Net::Json::Type::STRING)
		{
			/*
			* walk through the string and make sure there are no non-escaped double-qoutes
			*/
			for (int j = 1; j < value.length() - 1; ++j)
			{
				auto ec = refValue.get()[j];
				if (ec == '"')
				{
					if ((j - 1) < 0
						|| refValue.get()[j - 1] != '\\')
					{
						NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad string ... string contains double-qoutes that are not escaped ... double-qoutes inside a string must be escaped with '\\'"));
						return false;
					}
				}
			}

			// obtain the string from the json-string without the double-qoutes
			auto str = value.substr(1, value.length() - 2);
			this->push(str);

			return true;
		}
	}

	/*
	* check for boolean
	*/
	{
		if (Convert::is_boolean(value))
		{
			m_type = Net::Json::Type::BOOLEAN;
			this->push(Convert::ToBoolean(value));
			return true;
		}
	}

	/*
	* check for null value
	*/
	{
		if (!memcmp(refValue.get(), CSTRING("null"), 4))
		{
			m_type = Net::Json::Type::NULLVALUE;
			this->push(Net::Json::NullValue());
			return true;
		}
	}

	/*
	* check for number
	*/
	{
		m_type = Net::Json::Type::INTEGER;

		constexpr const char m_NumericPattern[] = "0123456789";
		for (size_t i = 0, dot = 0; i < value.size(); ++i)
		{
			auto c = refValue.get()[i];

			/*
			* check for decimal number
			*/
			if (c == '.')
			{
				if (dot > 0)
				{
					// number got multiplie decimal splitter
					NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad number ... number has more than one of the decimal ('.') splitter"));
					return false;
				}

				/*
				* decimal number
				*/
				++dot;

				m_type = Net::Json::Type::DOUBLE;
				continue;
			}

			bool m_isNumber = false;
			for (size_t z = 0; z < sizeof(m_NumericPattern); ++z)
			{
				if (static_cast<int>(c) == static_cast<int>(m_NumericPattern[z]))
				{
					m_isNumber = true;
				}
			}

			if (!m_isNumber)
			{
				/* not a number */

				/*
				* null value is handled above, so if the code is reaching the end and the type is still null
				* then the value is not valid format
				*/
				m_type = Net::Json::Type::NULLVALUE;
				break;
			}
		}

		/* value is definitely a number */
		if (m_type != Net::Json::Type::NULLVALUE)
		{
			/*
			* value is a decimal number
			*/
			if (m_type == Net::Json::Type::DOUBLE)
			{
				/*
				* determinate if it is worth a double or just a float
				*/

				if (Convert::is_double(value))
				{
					m_type = Net::Json::Type::DOUBLE;
				}
				else if (Convert::is_float(value))
				{
					m_type = Net::Json::Type::FLOAT;
				}
			}

			switch (m_type)
			{
			case Net::Json::Type::INTEGER:
				this->push(Convert::ToInt32(value));
				break;

			case Net::Json::Type::DOUBLE:
				this->push(Convert::ToDouble(value));
				break;

			case Net::Json::Type::FLOAT:
				this->push(Convert::ToFloat(value));
				break;
			}

			return true;
		}
	}

	NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad value ... value is from unknown type ... got '%s'"), refValue.get());
	return false;
}

/*
* @Todo: Add better parsing error detection
* How it is working in its current state
*	- reading object and parsing it using the object deserializer
*	- reading array and parsing it using recursive method
*	- reading string
*	- reading anything
*		- converting anything into int, float, double, boolean and null value
*/
bool Net::Json::Array::Deserialize(Net::String json, bool m_removedWhitespace)
{
	/*
	* remove whitespace and line breaks
	*/
	if (!m_removedWhitespace)
	{
		json.eraseAll(' ');
		json.eraseAll('\n');
		m_removedWhitespace = !m_removedWhitespace;
	}

	/*
	* Since we are passing the json string into an array deserializer
	* we will check for the syntax that does define an array in the json language at the beginning and end
	* if not then abort the parsing
	*/
	if (json.get().get()[0] != '[')
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Unexpected character in the beginning of the string ... got '%c' ... expected '['"), json.get().get()[0]);
		this->Free();
		return false;
	}

	if (json.get().get()[json.length() - 1] != ']')
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Unexpected character in the ending of the string ... got '%c' ... expected ']'"), json.get().get()[json.length() - 1]);
		this->Free();
		return false;
	}

	/*
	* Array's elements in json are split by the syntax (',')
	* iterate through the json string
	* read the elements and pass them into DeserializeAny method for further analysis
	*/
	uint8_t flag = 0;
	size_t v = 0; // begin for substr
	size_t arr_count = 0;
	size_t obj_count = 0;

	flag |= (int)EDeserializeFlag::FLAG_READING_ELEMENT;
	v = 0;

	auto ref = json.get();
	for (size_t i = 1; i < json.length() - 1; ++i)
	{
		auto c = ref.get()[i];

		if (flag & (int)EDeserializeFlag::FLAG_READING_ELEMENT)
		{
			/*
			* Spotted beginning of an object
			* now read it fully before further analysing
			*/
			if (c == '{')
			{
				++obj_count;
				flag |= (int)EDeserializeFlag::FLAG_READING_OBJECT;
			}
			else if (c == '}')
			{
				if (!(flag & (int)EDeserializeFlag::FLAG_READING_OBJECT)
					|| obj_count == 0)
				{
					// we got an ending curly for an object but missing the start curly
					NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad syntax ... expected '{' ... got '}'"));
					this->Free();
					return false;
				}

				--obj_count;
				flag &= ~(int)EDeserializeFlag::FLAG_READING_OBJECT;
			}

			/*
			* Spotted beginning of an array
			* now read it fully before further analysing
			*/
			if (c == '[')
			{
				++arr_count;
				flag |= (int)EDeserializeFlag::FLAG_READING_ARRAY;
			}
			else if (c == ']')
			{
				if (!(flag & (int)EDeserializeFlag::FLAG_READING_ARRAY)
					|| arr_count == 0)
				{
					// we got an ending curly for an arry but missing the start curly
					NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad syntax ... expected '[' ... got ']'"));
					this->Free();
					return false;
				}

				--arr_count;
				flag &= ~(int)EDeserializeFlag::FLAG_READING_ARRAY;
			}

			/*
			* Spotted beginning of a string
			* now read it fully
			*/
			if (c == '"'
				&& !(flag & (int)EDeserializeFlag::FLAG_READING_OBJECT
					|| flag & (int)EDeserializeFlag::FLAG_READING_ARRAY))
			{
				if (flag & (int)EDeserializeFlag::FLAG_READING_STRING)
				{
					flag &= ~(int)EDeserializeFlag::FLAG_READING_STRING;
				}
				else
				{
					flag |= (int)EDeserializeFlag::FLAG_READING_STRING;
				}
			}

			/*
			* check for the syntax that splits the elements in an array
			* or check if we have reached eof (end of file)
			*	will happen if there is only one element
			*/
			if (c == ',' || (i == json.size() - 3))
			{
				/*
				* keep going until we read the entire object or array
				* only stop if we reached eof
				*/
				if (c == ','
					&& (flag & (int)EDeserializeFlag::FLAG_READING_OBJECT 
						|| flag & (int)EDeserializeFlag::FLAG_READING_ARRAY
						|| flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					continue;
				}

				Net::String element = (i == json.size() - 3) ? json.substr(v + 1, i - v) : json.substr(v + 1, i - v - 1);

				/*
				* pass the value to the DeserializeAny method
				* that method will take any value and further analyse its type
				* it will use recursive to deserialize further object's or array's
				*/
				if (!DeserializeAny(element, m_removedWhitespace))
				{
					// no need to display error message, the method will do it
					this->Free();
					return false;
				}

				v = i;
			}
		}
	}

	if (arr_count > 0)
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad syntax ... expected an array ending curly ']'"));
		this->Free();
		return false;
	}

	if (obj_count > 0)
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad syntax ... expected an object ending curly '}'"));
		this->Free();
		return false;
	}

	return true;
}

bool Net::Json::Array::Parse(Net::String json)
{
	return this->Deserialize(json);
}

Net::Json::Document::Document()
{
	this->Init();
}

Net::Json::Document::~Document()
{
	this->Clear();
}

Net::Json::Type Net::Json::Document::GetType()
{
	return this->m_type;
}

Net::Json::Object Net::Json::Document::GetRootObject()
{
	return this->root_obj;
}

Net::Json::Array Net::Json::Document::GetRootArray()
{
	return this->root_array;
}

void Net::Json::Document::SetFreeRootObject(bool m_free)
{
	this->m_free_root_obj = m_free;
}

void Net::Json::Document::SetFreeRootArray(bool m_free)
{
	this->m_free_root_array = m_free;
}

void Net::Json::Document::Init()
{
	/* by default its an object */
	this->root_obj = { true };
	this->root_array = { true };
	this->m_free_root_obj = true;
	this->m_free_root_array = true;
	this->m_type = Type::OBJECT;
}

void Net::Json::Document::Clear()
{
	if (this->m_free_root_obj) this->root_obj.Free();
	if (this->m_free_root_array) this->root_array.Free();
}

Net::Json::BasicValueRead Net::Json::Document::operator[](const char* key)
{
	if (this->m_type != Net::Json::Type::OBJECT) return { nullptr };
	return this->root_obj.At(key);
}

Net::Json::BasicValueRead Net::Json::Document::operator[](int idx)
{
	if (this->m_type != Net::Json::Type::ARRAY) return { nullptr };
	return this->root_array.at(idx);
}

Net::Json::BasicValueRead Net::Json::Document::At(const char* key)
{
	if (this->m_type != Net::Json::Type::OBJECT) return { nullptr };
	return this->root_obj.At(key);
}

Net::Json::BasicValueRead Net::Json::Document::At(int idx)
{
	if (this->m_type != Net::Json::Type::ARRAY) return { nullptr };
	return this->root_array.at(idx);
}

void Net::Json::Document::Set(Object obj)
{
	this->root_obj = obj;
	this->m_type = Type::OBJECT;
}

void Net::Json::Document::Set(Object* obj)
{
	this->root_obj = *obj;
	this->m_type = Type::OBJECT;
}

void Net::Json::Document::Set(Array arr)
{
	this->root_array = arr;
	this->m_type = Type::ARRAY;
}

void Net::Json::Document::Set(Array* arr)
{
	this->root_array = *arr;
	this->m_type = Type::ARRAY;
}

Net::String Net::Json::Document::Serialize(SerializeType type)
{
	switch (this->m_type)
	{
	case Type::OBJECT:
		return this->root_obj.Serialize(type);

	case Type::ARRAY:
		return this->root_array.Serialize(type);

	default:
		break;
	}

	return "";
}

Net::String Net::Json::Document::Stringify(SerializeType type)
{
	return this->Serialize(type);
}

bool Net::Json::Document::Deserialize(Net::String json)
{
	/* re-init */
	this->Clear();
	this->Init();

	if (json.get().get()[0] == '{' && json.get().get()[json.length() - 1] == '}')
	{
		/* is object */
		this->m_type = Net::Json::Type::OBJECT;
		return this->root_obj.Deserialize(json);
	}
	else if (json.get().get()[0] == '[' && json.get().get()[json.length() - 1] == ']')
	{
		this->m_type = Net::Json::Type::ARRAY;
		return this->root_array.Deserialize(json);
	}

	/* error */
	// @todo: add error message
	return false;
}

bool Net::Json::Document::Parse(Net::String json)
{
	return this->Deserialize(json);
}

template class Net::Json::BasicValue<int>;
template class Net::Json::BasicValue<float>;
template class Net::Json::BasicValue<double>;
template class Net::Json::BasicValue<char*>;
template class Net::Json::BasicValue<const char*>;
template class Net::Json::BasicValue<Net::Json::Object>;
template class Net::Json::BasicValue<Net::Json::Array>;
template class Net::Json::BasicValue<Net::Json::NullValue>;
