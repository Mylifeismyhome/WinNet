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
	Net::String out(CSTRING(""));

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

			out.append(CSTRING(R"("%s" : %s)"), tmp->Key(), CSTRING("null"));
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

			out.append(CSTRING(R"("%s" : "%s")"), tmp->Key(), tmp->as_string());
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

			out.append(CSTRING(R"("%s" : %i)"), tmp->Key(), tmp->as_int());
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

			out.append(CSTRING(R"("%s" : %F)"), tmp->Key(), tmp->as_float());
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

			out.append(CSTRING(R"("%s" : %F)"), tmp->Key(), tmp->as_double());
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

			out.append(CSTRING(R"("%s" : %s)"), tmp->Key(), tmp->as_boolean() ? CSTRING("true") : CSTRING("false"));
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

			out.append(CSTRING(R"("%s" : )"), tmp->Key());
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

			out.append(CSTRING(R"("%s" : )"), tmp->Key());
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
bool Net::Json::Object::Deserialize(Net::String json)
{
	Vector<char*> object_chain = {};
	auto ret = this->Deserialize(json, object_chain);
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

/* actual deserialization */
bool Net::Json::Object::DeserializeAny(Net::String& key, Net::String& value, Vector<char*>& object_chain)
{
	// object detected
	if (value.get().get()[0] == '{' && value.get().get()[value.length() - 1] == '}')
	{
#ifdef BUILD_LINUX
		object_chain.push_back(strdup(key.get().get()));
#else
		object_chain.push_back(_strdup(key.get().get()));
#endif

		if (!this->Deserialize(value, object_chain))
			return false;

		object_chain.erase(object_chain.size() - 1);

		return true;
	}

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

	// array detected
	if (value.get().get()[0] == '[' && value.get().get()[value.length() - 1] == ']')
	{
		Net::Json::Array arr(true);
		if (!arr.Deserialize(value))
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

	// we have to figure out what kind of type the data is from
	// we start with treating it as an integer
	Net::Json::Type type = Net::Json::Type::INTEGER;

	if (Convert::is_boolean(value))
	{
		type = Net::Json::Type::BOOLEAN;
	}
	else if (!memcmp(value.get().get(), CSTRING("null"), 4))
	{
		type = Net::Json::Type::NULLVALUE;
	}
	else
	{
		// find out if there is any string or floating number to read

		uint8_t flag = 0;
		size_t v = 0; // begin for substr
		auto ref = value.get();
		for (size_t i = 0; i < value.size(); ++i)
		{
			auto c = ref.get()[i];

			if ((flag & (int)EDeserializeFlag::FLAG_READING_STRING))
			{
				// end of string reading
				if (c == '"')
				{
					flag &= ~(int)EDeserializeFlag::FLAG_READING_STRING;
					type = Net::Json::Type::STRING;

					auto str = value.substr(v + 1, i - v - 1);

					if (object_chain.size() > 0)
					{
						obj[key.get().get()] = str;
					}
					else
					{
						this->operator[](key.get().get()) = str;
					}

					return true; // early exit because there won't be anything left to read
				}

				// keep reading the string
				continue;
			}
			else
			{
				// check if there is any string to read
				if (c == '"')
				{
					flag |= (int)EDeserializeFlag::FLAG_READING_STRING;
					v = i;
				}

				// check if there is a dot that identifies a floating number
				if (c == '.')
				{
					if (Convert::is_double(value))
					{
						type = Net::Json::Type::DOUBLE;
					}
					else if (Convert::is_float(value))
					{
						type = Net::Json::Type::FLOAT;
					}

					break;
				}
			}
		}
	}

	switch (type)
	{
	case Net::Json::Type::INTEGER:
		if (object_chain.size() > 0)
		{
			obj[key.get().get()] = Convert::ToInt32(value);
			break;
		}

		this->operator[](key.get().get()) = Convert::ToInt32(value);
		break;

	case Net::Json::Type::BOOLEAN:
		if (object_chain.size() > 0)
		{
			obj[key.get().get()] = Convert::ToBoolean(value);
			break;
		}

		this->operator[](key.get().get()) = Convert::ToBoolean(value);
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

	case Net::Json::Type::NULLVALUE:
		if (object_chain.size() > 0)
		{
			obj[key.get().get()] = Net::Json::NullValue();
			break;
		}

		this->operator[](key.get().get()) = Net::Json::NullValue();
		break;

	default:
		NET_LOG_ERROR(CSTRING("INVALID TYPE"));
		return false;
	}

	return true;
}

/*
* @Todo: Add better parsing error detection
* How it is working in its current state
*	- reading till the common seperator ':' reached
*		- then extracting the key & value from each other
*	- processing the value
*		- using recursive method to parse object chain
*		- creating an array object on parsing array
*/
bool Net::Json::Object::Deserialize(Net::String& json, Vector<char*>& object_chain)
{
	if (json.get().get()[0] != '{')
	{
		std::cout << "NOT AN OBJECT" << std::endl;
		return false;
	}

	if (json.get().get()[json.length() - 1] != '}')
	{
		std::cout << "NOT AN OBJECT2" << std::endl;
		return false;
	}

	uint8_t flag = 0;
	size_t v = 0; // begin for substr
	auto ref = json.get();

	Net::String lastKey;
	for (size_t i = 1; i < json.size() - 1; ++i)
	{
		auto c = ref.get()[i];

		if ((flag & (int)EDeserializeFlag::FLAG_READING_KEY))
		{
			// read till we reach the common splitter for an object
			if (c == ':')
			{
				// read the key
				size_t kb = 0; // begin
				size_t ke = 0; // end
				for (size_t j = v; j < i; ++j)
				{
					auto d = ref.get()[j];
					if (d == '"')
					{
						if (kb != 0)
						{
							ke = j;
							break;
						}
						else
						{
							kb = j;
						}
					}
				}

				if (!kb || !ke)
				{
					// @todo: add error message {invalid key}
					return false;
				}

				lastKey = json.substr(kb + 1, ke - kb - 1);

				flag &= ~(int)EDeserializeFlag::FLAG_READING_KEY;
				flag |= (int)EDeserializeFlag::FLAG_READING_VALUE;

				v = i + 1;
			}
		}
		else if ((flag & (int)EDeserializeFlag::FLAG_READING_VALUE))
		{
			if ((flag & (int)EDeserializeFlag::FLAG_READING_ARRAY))
			{
				// end of reading arry
				if (c == ']')
				{
					flag &= ~(int)EDeserializeFlag::FLAG_READING_ARRAY;
					flag &= ~(int)EDeserializeFlag::FLAG_READING_VALUE;

					Net::String value = json.substr(v, i - v + 1);
					if (!DeserializeAny(lastKey, value, object_chain))
					{
						return false;
					}
				}
			}
			else if ((flag & (int)EDeserializeFlag::FLAG_READING_OBJECT))
			{
				// end of reading object
				if (c == '}')
				{
					flag &= ~(int)EDeserializeFlag::FLAG_READING_OBJECT;
					flag &= ~(int)EDeserializeFlag::FLAG_READING_VALUE;

					Net::String value = json.substr(v, i - v + 1);
					if (!DeserializeAny(lastKey, value, object_chain))
					{
						return false;
					}
				}
			}
			else
			{
				// detected an array, read it
				if (c == '[')
				{
					flag |= (int)EDeserializeFlag::FLAG_READING_ARRAY;
				}
				// detected an object, read it
				else if (c == '{')
				{
					flag |= (int)EDeserializeFlag::FLAG_READING_OBJECT;
				}
				// read till we reach the next seperator
				else if (c == ',')
				{
					flag &= ~(int)EDeserializeFlag::FLAG_READING_VALUE;

					Net::String value = json.substr(v, i - v);
					if (!DeserializeAny(lastKey, value, object_chain))
					{
						return false;
					}
				}
				// or read till we reach the end
				else if (i == json.length() - 2)
				{
					flag &= ~(int)EDeserializeFlag::FLAG_READING_VALUE;

					Net::String value = json.substr(v, i - v + 1);
					if (!DeserializeAny(lastKey, value, object_chain))
					{
						return false;
					}
				}
			}
		}
		else
		{
			// read key
			flag |= (int)EDeserializeFlag::FLAG_READING_KEY;
			v = i;
		}
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
	Net::String out;

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

			out += Net::String(CSTRING(R"("%s")"), tmp->as_string());
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

	//size_t it = 0;
	//if ((it = out.findLastOf(CSTRING(","))) != NET_STRING_NOT_FOUND)
	//	out.erase(it, 1);

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

bool Net::Json::Array::DeserializeAny(Net::String& str)
{
	// we have to figure out what kind of type the data is from
	// we start with treating it as an integer
	Net::Json::Type type = Net::Json::Type::INTEGER;

	if (Convert::is_boolean(str))
	{
		type = Net::Json::Type::BOOLEAN;
	}
	else if (!memcmp(str.get().get(), CSTRING("null"), 4))
	{
		type = Net::Json::Type::NULLVALUE;
	}
	else
	{
		// make sure we are having a floating number
		for (size_t i = 0; i < str.size(); ++i)
		{
			auto c = str.get().get()[i];
			if (c == '.')
			{
				if (Convert::is_double(str))
				{
					type = Net::Json::Type::DOUBLE;
				}
				else if (Convert::is_float(str))
				{
					type = Net::Json::Type::FLOAT;
				}

				break;
			}
		}
	}

	switch (type)
	{
	case Net::Json::Type::INTEGER:
		this->push(Convert::ToInt32(str));
		break;

	case Net::Json::Type::BOOLEAN:
		this->push(Convert::ToBoolean(str));
		break;

	case Net::Json::Type::DOUBLE:
		this->push(Convert::ToDouble(str));
		break;

	case Net::Json::Type::FLOAT:
		this->push(Convert::ToFloat(str));
		break;

	case Net::Json::Type::NULLVALUE:
		this->push(Net::Json::NullValue());
		break;

	default:
		NET_LOG_ERROR(CSTRING("INVALID TYPE"));
		return false;
	}

	return true;
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
bool Net::Json::Array::Deserialize(Net::String json)
{
	if (json.get().get()[0] != '[')
	{
		std::cout << "NOT ARRAY" << std::endl;
		/* not an array */
		return false;
	}

	if (json.get().get()[json.length() - 1] != ']')
	{
		std::cout << "NOT ARRAY2" << std::endl;
		/* not an array */
		return false;
	}

	uint8_t flag = 0;
	size_t v = 0; // begin for substr
	auto ref = json.get();
	for (size_t i = 1; i < json.length() - 1; ++i)
	{
		auto c = ref.get()[i];

		if ((flag & (int)EDeserializeFlag::FLAG_READING_OBJECT))
		{
			// end of object reading
			if (c == '}')
			{
				flag &= ~(int)EDeserializeFlag::FLAG_READING_OBJECT;

				auto object = json.substr(v, i - v + 1);
				Net::Json::Object obj(true);
				if (!obj.Deserialize(object))
				{
					/* error */
					std::cout << "UNABEL TO DESERIALIZE OBJECT" << std::endl;
					return false;
				}
				this->push(obj);
			}

			// keep reading the object
			continue;
		}
		else if ((flag & (int)EDeserializeFlag::FLAG_READING_ARRAY))
		{
			// end of array reading
			if (c == ']')
			{
				flag &= ~(int)EDeserializeFlag::FLAG_READING_ARRAY;

				auto array = json.substr(v, i - v + 1);
				Net::Json::Array arr(true);
				if (!arr.Deserialize(array))
				{
					/* error */
					std::cout << "UNABEL TO DESERIALIZE ARRAY" << std::endl;
					return false;
				}
				this->push(arr);
			}

			// keep reading the array
			continue;
		}
		else if ((flag & (int)EDeserializeFlag::FLAG_READING_STRING))
		{
			// end of string reading
			if (c == '"')
			{
				flag &= ~(int)EDeserializeFlag::FLAG_READING_STRING;

				auto str = json.substr(v + 1, i - v - 1);
				this->push(str);
			}

			// keep reading the string
			continue;
		}
		else if ((flag & (int)EDeserializeFlag::FLAG_READING_ANY))
		{
			// read anything till we reach a seperator
			if (c == ',')
			{
				flag &= ~(int)EDeserializeFlag::FLAG_READING_ANY;

				Net::String str = json.substr(v, i - v);

				if (!DeserializeAny(str))
				{
					return false;
				}
			}
			// or read anything till we reach the end
			else if (i == json.length() - 2)
			{
				flag &= ~(int)EDeserializeFlag::FLAG_READING_ANY;

				Net::String str = json.substr(v, i - v + 1);

				if (!DeserializeAny(str))
				{
					return false;
				}
			}

			// keep reading
			continue;
		}
		else
		{
			// check if there is any object to read
			if (c == '{')
			{
				flag |= (int)EDeserializeFlag::FLAG_READING_OBJECT;
				v = i;
				continue;
			}

			// check if there is any array to read
			else if (c == '[')
			{
				flag |= (int)EDeserializeFlag::FLAG_READING_ARRAY;
				v = i;
				continue;
			}

			// check if there is any string to read
			else if (c == '"')
			{
				flag |= (int)EDeserializeFlag::FLAG_READING_STRING;
				v = i;
				continue;
			}

			// we are neither reading an object, array or string
			// so we do read anything untill we can process it
			flag |= (int)EDeserializeFlag::FLAG_READING_ANY;
			v = (c == ',') ? i + 1 : i;

			// check if we reached the end
			if (i == json.length() - 2)
			{
				flag &= ~(int)EDeserializeFlag::FLAG_READING_ANY;

				Net::String str = json.substr(v, i - v + 1);

				if (!DeserializeAny(str))
				{
					return false;
				}
			}
		}
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
