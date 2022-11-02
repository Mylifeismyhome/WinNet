/*
	Author: Tobias Staack
*/
#include <Net/Net/NetJson.h>
#include <Net/Cryption/XOR.h>
#include <Net/assets/manager/logmanager.h>

constexpr auto Net_Json_Boolean_True = "true";
constexpr auto Net_Json_Boolean_False = "false";

namespace Net
{
	namespace Json
	{
		/*
		* this method will prepare a json string before parsing
		* it will remove all the whitespaces outside of string's
		* it will remove all the line breaks
		*/
		static bool PrepareString(Net::String& json)
		{
			uint8_t flag = 0;

			std::vector<size_t> m_IndexCharacterToSkip = {};
			for (size_t i = 0; i < json.size(); ++i)
			{
				auto c = json[i];
				if (c == '"')
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
				* remove any white space outside of string's
				*/
				else if (c == ' '
					&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					// push back the index of the character we will skip
					m_IndexCharacterToSkip.push_back(i);
				}
				/*
				* remove line break
				*/
				else if (c == '\n'
					&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					// push back the index of the character we will skip
					m_IndexCharacterToSkip.push_back(i);
				}
				/*
				* remove set position to start of line
				*/
				else if (c == '\r'
					&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					// push back the index of the character we will skip
					m_IndexCharacterToSkip.push_back(i);
				}
				/*
				* remove tabulator
				*/
				else if (c == '\t'
					&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					// push back the index of the character we will skip
					m_IndexCharacterToSkip.push_back(i);
				}
				else if (c == '\b'
					&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					// push back the index of the character we will skip
					m_IndexCharacterToSkip.push_back(i);
				}
				else if (c == '\f'
					&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					// push back the index of the character we will skip
					m_IndexCharacterToSkip.push_back(i);
				}
			}

			if (!json.eraseAll(m_IndexCharacterToSkip))
			{
				return false;
			}

			return true;
		}

		static bool PrepareString(Net::ViewString& json)
		{
			if (!json.original())
			{
				return false;
			}

			uint8_t flag = 0;

			std::vector<size_t> m_IndexCharacterToSkip = {};
			for (size_t i = json.start(); i < json.end(); ++i)
			{
				auto c = json[i];
				if (c == '"')
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
				* remove any white space outside of string's
				*/
				else if (c == ' '
					&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					// push back the index of the character we will skip
					m_IndexCharacterToSkip.push_back(i);
				}
				/*
				* remove line break
				*/
				else if (c == '\n'
					&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					// push back the index of the character we will skip
					m_IndexCharacterToSkip.push_back(i);
				}
				/*
				* remove set position to start of line
				*/
				else if (c == '\r'
					&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					// push back the index of the character we will skip
					m_IndexCharacterToSkip.push_back(i);
				}
				/*
				* remove tabulator
				*/
				else if (c == '\t'
					&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					// push back the index of the character we will skip
					m_IndexCharacterToSkip.push_back(i);
				}
				else if (c == '\b'
					&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					// push back the index of the character we will skip
					m_IndexCharacterToSkip.push_back(i);
				}
				else if (c == '\f'
					&& !(flag & (int)EDeserializeFlag::FLAG_READING_STRING))
				{
					// push back the index of the character we will skip
					m_IndexCharacterToSkip.push_back(i);
				}
			}

			if (!reinterpret_cast<Net::String*>(json.original())->eraseAll(m_IndexCharacterToSkip))
			{
				return false;
			}

			return json.refresh();
		}

		static void EncodeString(Net::String& buffer)
		{
			if (buffer.empty()) return;

			Net::String out;
			out.reserve(buffer.size() * 2);

			for (size_t i = 0; i < buffer.size(); ++i)
			{
				auto c = buffer[i];
				switch (c)
				{
				case '\\':
					if ((i + 1) < buffer.size() &&
						buffer[i + 1] == '\\')
					{
						out.append(reinterpret_cast<const char*>(CSTRING(R"(\\)")));
						++i;
						continue;
					}

					out.append(reinterpret_cast<const char*>(CSTRING(R"(\\)")));
					break;

				case '"':
					out.append(reinterpret_cast<const char*>(CSTRING(R"(\")")));
					break;

				case '\b':
					out.append(reinterpret_cast<const char*>(CSTRING(R"(\b)")));
					break;

				case '\t':
					out.append(reinterpret_cast<const char*>(CSTRING(R"(\t)")));
					break;

				case '\n':
					out.append(reinterpret_cast<const char*>(CSTRING(R"(\n)")));
					break;

				case '\f':
					out.append(reinterpret_cast<const char*>(CSTRING(R"(\f)")));
					break;

				case '\r':
					out.append(reinterpret_cast<const char*>(CSTRING(R"(\r)")));
					break;

				default:
					out.append(c);
					break;
				}
			}

			out.finalize();
			buffer.move(out);
		}

		static Net::String EncodeString(Net::ViewString& vs)
		{
			Net::String out;
			out.reserve(vs.size() * 2);

			for (size_t i = vs.start(); i < vs.end(); ++i)
			{
				auto c = vs[i];
				switch (c)
				{
				case '\\':
					if ((i + 1) < vs.end() &&
						vs[i + 1] == '\\')
					{
						out.append(reinterpret_cast<const char*>(CSTRING(R"(\\)")));
						++i;
						continue;
					}

					out.append(reinterpret_cast<const char*>(CSTRING(R"(\\)")));
					break;

				case '"':
					out.append(reinterpret_cast<const char*>(CSTRING(R"(\")")));
					break;

				case '\b':
					out.append(reinterpret_cast<const char*>(CSTRING(R"(\b)")));
					break;

				case '\t':
					out.append(reinterpret_cast<const char*>(CSTRING(R"(\t)")));
					break;

				case '\n':
					out.append(reinterpret_cast<const char*>(CSTRING(R"(\n)")));
					break;

				case '\f':
					out.append(reinterpret_cast<const char*>(CSTRING(R"(\f)")));
					break;

				case '\r':
					out.append(reinterpret_cast<const char*>(CSTRING(R"(\r)")));
					break;

				default:
					out.append(c);
					break;
				}
			}

			out.finalize();
			return out;
		}

		static void DecodeString(Net::String& buffer)
		{
			if (buffer.size() < 2)
				return;

			Net::String out;
			out.reserve(buffer.size());

			auto ref = buffer.get();
			auto pBuffer = ref.get();
			for (size_t i = 0; i < buffer.size(); i++)
			{
				if (!memcmp(&pBuffer[i], CSTRING(R"(\")"), 2))
				{
					out.append('"');
					++i;
					continue;
				}
				else if (!memcmp(&pBuffer[i], CSTRING(R"(\/)"), 2))
				{
					out.append('\\');
					++i;
					continue;
				}
				else if (!memcmp(&pBuffer[i], CSTRING(R"(\\)"), 2))
				{
					out.append('\\');
					++i;
					continue;
				}
				else if (!memcmp(&pBuffer[i], CSTRING(R"(\b)"), 2))
				{
					out.append('\b');
					++i;
					continue;
				}
				else if (!memcmp(&pBuffer[i], CSTRING(R"(\t)"), 2))
				{
					out.append('\t');
					++i;
					continue;
				}
				else if (!memcmp(&pBuffer[i], CSTRING(R"(\n)"), 2))
				{
					out.append('\n');
					++i;
					continue;
				}
				else if (!memcmp(&pBuffer[i], CSTRING(R"(\f)"), 2))
				{
					out.append('\f');
					++i;
					continue;
				}
				else if (!memcmp(&pBuffer[i], CSTRING(R"(\r)"), 2))
				{
					out.append('\r');
					++i;
					continue;
				}

				out.append(pBuffer[i]);
			}

			out.finalize();
			buffer.move(out);
		}

		static Net::String DecodeString(Net::ViewString& vs)
		{
			Net::String out;
			out.reserve(vs.size());

			/*
			* early exit for size less than 2
			* or invalid
			*/
			if (vs.size() < 2 || vs.size() == INVALID_SIZE)
			{
				for (size_t i = vs.start(); i < vs.end(); i++)
				{
					out.append(vs[i]);
				}
				out.finalize();
				return out;
			}

			for (size_t i = vs.start(); i < vs.end(); i++)
			{
				if (!memcmp(&vs.get()[i], CSTRING(R"(\")"), 2))
				{
					out.append('"');
					++i;
					continue;
				}
				else if (!memcmp(&vs.get()[i], CSTRING(R"(\/)"), 2))
				{
					out.append('\\');
					++i;
					continue;
				}
				else if (!memcmp(&vs.get()[i], CSTRING(R"(\\)"), 2))
				{
					out.append('\\');
					++i;
					continue;
				}
				else if (!memcmp(&vs.get()[i], CSTRING(R"(\b)"), 2))
				{
					out.append('\b');
					++i;
					continue;
				}
				else if (!memcmp(&vs.get()[i], CSTRING(R"(\t)"), 2))
				{
					out.append('\t');
					++i;
					continue;
				}
				else if (!memcmp(&vs.get()[i], CSTRING(R"(\n)"), 2))
				{
					out.append('\n');
					++i;
					continue;
				}
				else if (!memcmp(&vs.get()[i], CSTRING(R"(\f)"), 2))
				{
					out.append('\f');
					++i;
					continue;
				}
				else if (!memcmp(&vs.get()[i], CSTRING(R"(\r)"), 2))
				{
					out.append('\r');
					++i;
					continue;
				}

				out.append(vs[i]);
			}

			out.finalize();
			return out;
		}
	}
}

int Net::Json::Convert::ToInt32(char* str)
{
	return std::stoi(str);
}

int Net::Json::Convert::ToInt32(Net::ViewString& vs)
{
	auto end = &vs.get()[vs.end()];
	return static_cast<int>(strtol(&vs.get()[vs.start()], &end, 10));
}

float Net::Json::Convert::ToFloat(char* str)
{
	return std::stof(str);
}

float Net::Json::Convert::ToFloat(Net::ViewString& vs)
{
	auto end = &vs.get()[vs.end()];
	return strtof(&vs.get()[vs.start()], &end);
}

double Net::Json::Convert::ToDouble(char* str)
{
	return std::stod(str);
}

double Net::Json::Convert::ToDouble(Net::ViewString& vs)
{
	auto end = &vs.get()[vs.end()];
	return strtod(&vs.get()[vs.start()], &end);
}

bool Net::Json::Convert::ToBoolean(char* str)
{
	if (!strcmp(Net_Json_Boolean_True, str))
		return true;
	else if (!strcmp(Net_Json_Boolean_False, str))
		return false;

	NET_LOG_ERROR(CSTRING("[Net::Json::Convert] - failed to convert to boolean"));
	return false;
}

bool Net::Json::Convert::ToBoolean(Net::ViewString& vs)
{
	if (!memcmp(&vs.get()[vs.start()], Net_Json_Boolean_True, vs.size()))
		return true;
	if (!memcmp(&vs.get()[vs.start()], Net_Json_Boolean_False, vs.size()))
		return false;

	NET_LOG_ERROR(CSTRING("[Net::Json::Convert] - failed to convert to boolean"));
	return false;
}

bool Net::Json::Convert::is_float(char* str)
{
	char* end = nullptr;
	double val = strtof(str, &end);
	return end != str && *end == '\0' && val != HUGE_VALF;
}

bool Net::Json::Convert::is_float(Net::ViewString& vs)
{
	auto end = &vs.get()[vs.end()];
	double val = strtof(&vs.get()[vs.start()], &end);
	return end != &vs.get()[vs.start()] && val != HUGE_VALF;
}

bool Net::Json::Convert::is_double(char* str)
{
	char* end = nullptr;
	double val = strtod(str, &end);
	return end != str && *end == '\0' && val != HUGE_VAL;
}

bool Net::Json::Convert::is_double(Net::ViewString& vs)
{
	auto end = &vs.get()[vs.end()];
	double val = strtod(&vs.get()[vs.start()], &end);
	return end != &vs.get()[vs.start()] && val != HUGE_VALF;
}

bool Net::Json::Convert::is_boolean(char* str)
{
	if (!strcmp(Net_Json_Boolean_True, str))
		return true;
	else if (!strcmp(Net_Json_Boolean_False, str))
		return true;

	return false;
}

bool Net::Json::Convert::is_boolean(Net::ViewString& vs)
{
	if (!memcmp(&vs.get()[vs.start()], Net_Json_Boolean_True, vs.size()))
		return true;
	if (!memcmp(&vs.get()[vs.start()], Net_Json_Boolean_False, vs.size()))
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
			FREE(cast->Value());
		}
	}

	this->value = {};
	FREE(this->key);
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
	this->key = ALLOC<char>(len + 1);
	if (!this->key) return;
	memcpy(this->key, key, len);
	this->key[len] = 0;
}

template <typename T>
void Net::Json::BasicValue<T>::SetKey(Net::ViewString& key)
{
	this->key = ALLOC<char>(key.size() + 1);
	if (!this->key) return;
	memcpy(this->key, &key.get()[key.start()], key.size());
	this->key[key.size()] = 0;
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

static Net::Json::BasicValueRead object_to_BasicValueRead(void* ptr, Net::ViewString& key)
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

Net::Json::BasicValueRead Net::Json::BasicValueRead::operator[](Net::ViewString& key)
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
	char* ptr = ALLOC<char>(len + 1);
	memcpy(ptr, value, len);
	ptr[len] = 0;

	auto cast = ((BasicValue<char*>*)this->ptr);
	if (cast->GetType() == Type::STRING
		&& cast->Value())
	{
		FREE(cast->Value());
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
	if (!bSharedMemory) this->Destroy();
}

void Net::Json::Object::Destroy()
{
	this->value.clear();
}

template<typename T>
bool Net::Json::Object::__append(const char* key, T value, Type type)
{
	BasicValue<T>* heap = ALLOC<BasicValue<T>>();
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

		auto lenKey = strlen(tmp->Key());
		auto lenKey2 = strlen(key);
		if (lenKey != lenKey2) continue;

		bool m_notMatch = false;
		for (size_t j = 0; j < lenKey; ++j)
		{
			char c1 = tmp->Key()[j];
			char c2 = key[j];
			if (c1 != c2)
			{
				m_notMatch = true;
				break;
			}
		}

		if (m_notMatch) continue;
		return (BasicValue<T>*)value[i];
	}

	return nullptr;
}

template <typename T>
Net::Json::BasicValue<T>* Net::Json::Object::__get(Net::ViewString& key)
{
	for (size_t i = 0; i < value.size(); ++i)
	{
		BasicValue<T>* tmp = (BasicValue<T>*)value[i];

		auto lenKey = strlen(tmp->Key());
		if (key.size() != lenKey) continue;

		bool m_notMatch = false;
		for (size_t j = 0; j < lenKey; ++j)
		{
			char c1 = tmp->Key()[j];
			char c2 = key.get()[key.start() + j];
			if (c1 != c2)
			{
				m_notMatch = true;
				break;
			}
		}

		if (m_notMatch) continue;
		return (BasicValue<T>*)value[i];
	}

	return nullptr;
}

Net::Json::BasicValueRead Net::Json::Object::operator[](const char* key)
{
	return this->At(key);
}

Net::Json::BasicValueRead Net::Json::Object::operator[](Net::ViewString& key)
{
	return this->At(key);
}

Net::Json::BasicValueRead Net::Json::Object::At(const char* key)
{
	auto ptr = this->__get<Object>(key);
	if (!ptr)
	{
		BasicValue<Object>* heap = ALLOC<BasicValue<Object>>();
		if (!heap) return { nullptr };
		heap->SetType(Type::OBJECT);
		heap->SetKey(key);
		this->__push(heap);
		return { heap };
	}

	return { ptr };
}

Net::Json::BasicValueRead Net::Json::Object::At(Net::ViewString& key)
{
	auto ptr = this->__get<Object>(key);
	if (!ptr)
	{
		BasicValue<Object>* heap = ALLOC<BasicValue<Object>>();
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
	char* ptr = ALLOC<char>(len + 1);
	memcpy(ptr, value, len);
	ptr[len] = 0;
	if (!__append(key, ptr, Type::STRING))
	{
		FREE(ptr);
		return false;
	}

	return true;
}

bool Net::Json::Object::Append(const char* key, Object value)
{
	return __append(key, value, Type::OBJECT);
}

size_t Net::Json::Object::CalcLengthForSerialize()
{
	size_t m_size = 0;
	for (size_t i = 0; i < value.size(); ++i)
	{
		auto tmp = (BasicValue<void*>*)value[i];
		if (tmp->GetType() == Type::NULLVALUE)
		{
			m_size += NET_JSON_ARR_LEN("null");
		}
		else if (tmp->GetType() == Type::STRING)
		{
			auto len = strlen(tmp->as_string());
			m_size += len;
		}
		else if (tmp->GetType() == Type::INTEGER)
		{
			if (tmp->as_int() == 0) m_size += 1;
			else m_size += floor(log10(abs(tmp->as_int()))) + 1;
		}
		else if (tmp->GetType() == Type::FLOAT)
		{
			int digits = 0;
			long cnum = tmp->as_float();
			while (cnum > 0) //count no of digits before floating point
			{
				digits++;
				cnum = cnum / 10;
			}
			if (tmp->as_float() == 0)
				digits = 1;

			double no_float;
			no_float = tmp->as_float() * (pow(10, (8 - digits)));
			long long int total = (long long int)no_float;
			int no_of_digits, extrazeroes = 0;
			for (int i = 0; i < 8; i++)
			{
				int dig;
				dig = total % 10;
				total = total / 10;
				if (dig != 0)
					break;
				else
					extrazeroes++;
			}
			no_of_digits = 8 - extrazeroes;
			m_size += no_of_digits;
		}
		else if (tmp->GetType() == Type::DOUBLE)
		{
			int digits = 0;
			long cnum = tmp->as_double();
			while (cnum > 0) //count no of digits before floating point
			{
				digits++;
				cnum = cnum / 10;
			}
			if (tmp->as_double() == 0)
				digits = 1;

			double no_float;
			no_float = tmp->as_double() * (pow(10, (16 - digits)));
			long long int total = (long long int)no_float;
			int no_of_digits, extrazeroes = 0;
			for (int i = 0; i < 16; i++)
			{
				int dig;
				dig = total % 10;
				total = total / 10;
				if (dig != 0)
					break;
				else
					extrazeroes++;
			}
			no_of_digits = 16 - extrazeroes;
			m_size += no_of_digits;
		}
		else if (tmp->GetType() == Type::BOOLEAN)
		{
			if (tmp->as_boolean())
			{
				m_size += NET_JSON_ARR_LEN("true");
			}
			else
			{
				m_size += NET_JSON_ARR_LEN("false");
			}
		}
		else if (tmp->GetType() == Type::OBJECT)
		{
			m_size += tmp->as_object()->CalcLengthForSerialize();
		}
		else if (tmp->GetType() == Type::ARRAY)
		{
			m_size += tmp->as_array()->CalcLengthForSerialize();
		}
	}
	return m_size;
}

Net::String Net::Json::Object::Serialize(SerializeType type)
{
	Net::Json::SerializeT st;
	st.m_reserved = false;
	if (!this->TrySerialize(type, st))
	{
		return {};
	}
	st.m_buffer.finalize();
	return st.m_buffer;
}

Net::String Net::Json::Object::Stringify(SerializeType type)
{
	return this->Serialize(type);
}

/* wrapper */
bool Net::Json::Object::Deserialize(Net::String& json, bool m_prepareString)
{
	Vector<char*> object_chain = {};
	return this->Deserialize(json, object_chain, m_prepareString);
}

bool Net::Json::Object::Deserialize(Net::ViewString& vs, bool m_prepareString)
{
	Vector<Net::ViewString*> object_chain = {};
	return this->Deserialize(vs, object_chain, m_prepareString);
}

bool Net::Json::Object::Deserialize(Net::String json)
{
	return this->Deserialize(json, false);
}

bool Net::Json::Object::Deserialize(Net::ViewString& json)
{
	return this->Deserialize(json, false);
}

bool Net::Json::Object::Parse(Net::String json)
{
	return this->Deserialize(json, false);
}

bool Net::Json::Object::Parse(Net::ViewString& json)
{
	return this->Deserialize(json, false);
}

/*
* this method will validate the value
* also, it will push it to the memory
* it uses recursive calls to deserialize children from object's and array's
*/
bool Net::Json::Object::DeserializeAny(Net::String& key, Net::String& value, Vector<char*>& object_chain, bool m_prepareString)
{
	Net::Json::Type m_type = Net::Json::Type::NULLVALUE;

	auto keyRef = key.get();
	auto pKey = keyRef.get();

	auto valueRef = value.get();
	auto pValue = valueRef.get();

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
		if (value[0] == '{')
		{
			m_type = Net::Json::Type::OBJECT;
		}

		if (value[value.length()] == '}'
			&& m_type != Net::Json::Type::OBJECT)
		{
			// we got an ending curly for an object, but missing the starting curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad object ... got an ending curly for an object, but missing the starting curly ... '%s'"), pValue);
			return false;
		}
		else if (value[value.length()] != '}'
			&& m_type == Net::Json::Type::OBJECT)
		{
			// we got a starting curly for an object, but missing the ending curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad object ... got a starting curly for an object, but missing the ending curly ... '%s'"), pValue);
			return false;
		}
		else if (m_type == Net::Json::Type::OBJECT)
		{
			// object seem to be fine, now call it's deserializer
			object_chain.push_back(pKey);
			{
				// need this line otherwise empty objects do not work
				if (object_chain.size() > 0)
				{
					obj[pKey] = ALLOC<Net::Json::Object>();
				}
				else
				{
					this->operator[](pKey) = ALLOC<Net::Json::Object>();
				}

				if (!this->Deserialize(value, object_chain, m_prepareString))
					return false;
			}
			object_chain.erase(object_chain.size() - 1);

			return true;
		}
	}

	/*
	* check for array
	*/
	{
		if (value[0] == '[')
		{
			m_type = Net::Json::Type::ARRAY;
		}

		if (value[value.length()] == ']'
			&& m_type != Net::Json::Type::ARRAY)
		{
			// we got an ending curly for an array, but missing the starting curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad array ... got an ending curly for an array, but missing the starting curly ... '%s'"), pValue);
			return false;
		}
		else if (value[value.length()] != ']'
			&& m_type == Net::Json::Type::ARRAY)
		{
			// we got a starting curly for an array, but missing the ending curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad array ... got a starting curly for an array, but missing the ending curly ... '%s'"), pValue);
			return false;
		}
		else if (m_type == Net::Json::Type::ARRAY)
		{
			// array seem to be fine, now call it's deserializer
			{
				Net::Json::Array arr(true);
				if (!arr.Deserialize(value, m_prepareString))
				{
					// @todo: add logging
					return false;
				}

				if (object_chain.size() > 0)
				{
					obj[pKey] = arr;
				}
				else
				{
					this->operator[](pKey) = arr;
				}
			}

			return true;
		}
	}

	/*
	* check for string
	*/
	{
		if (value[0] == '"')
		{
			m_type = Net::Json::Type::STRING;
		}

		if (value[value.length()] == '"'
			&& m_type != Net::Json::Type::STRING)
		{
			// we got an ending double-qoute for a string, but missing the starting
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad string ... got an ending double-qoute for a string, but missing the starting ... '%s'"), pValue);
			return false;
		}
		else if (value[value.length()] != '"'
			&& m_type == Net::Json::Type::STRING)
		{
			// we got a starting double-qoute for a string, but missing the ending
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad string ... got a starting double-qoute for a string, but missing the ending ... '%s'"), pValue);
			return false;
		}
		else if (m_type == Net::Json::Type::STRING)
		{
			/*
			* walk through the string and make sure there are no non-escaped double-qoutes
			*/
			for (int j = 1; j < value.length() - 1; ++j)
			{
				auto ec = value[j];
				if (ec == '"')
				{
					if ((j - 1) < 0
						|| value[j - 1] != '\\')
					{
						NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad string ... string contains double-qoutes that are not escaped ... double-qoutes inside a string must be escaped with '\\'"));
						return false;
					}
				}
			}

			// obtain the string from the json-string without the double-qoutes
			Net::String str(value.substr(1, value.length() - 1));
			Net::Json::DecodeString(str);

			if (object_chain.size() > 0)
			{
				obj[pKey] = str.get().get();
			}
			else
			{
				this->operator[](pKey) = str.get().get();
			}

			return true;
		}
	}

	/*
	* check for boolean
	*/
	{
		if (Convert::is_boolean(pValue))
		{
			m_type = Net::Json::Type::BOOLEAN;

			if (object_chain.size() > 0)
			{
				obj[pKey] = Convert::ToBoolean(pValue);
				return true;
			}

			this->operator[](pKey) = Convert::ToBoolean(pValue);
			return true;
		}
	}

	/*
	* check for null value
	*/
	{
		if (!memcmp(pValue, CSTRING("null"), strlen(pValue)))
		{
			m_type = Net::Json::Type::NULLVALUE;

			if (object_chain.size() > 0)
			{
				obj[pKey] = Net::Json::NullValue();
				return true;
			}

			this->operator[](pKey) = Net::Json::NullValue();
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
			auto c = value[i];

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
					break;
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

				if (Convert::is_double(pValue))
				{
					m_type = Net::Json::Type::DOUBLE;
				}
				else if (Convert::is_float(pValue))
				{
					m_type = Net::Json::Type::FLOAT;
				}
			}

			switch (m_type)
			{
			case Net::Json::Type::INTEGER:
				if (object_chain.size() > 0)
				{
					obj[pKey] = Convert::ToInt32(pValue);
					break;
				}

				this->operator[](pKey) = Convert::ToInt32(pValue);
				break;

			case Net::Json::Type::DOUBLE:
				if (object_chain.size() > 0)
				{
					obj[pKey] = Convert::ToDouble(pValue);
					break;
				}

				this->operator[](pKey) = Convert::ToDouble(pValue);
				break;

			case Net::Json::Type::FLOAT:
				if (object_chain.size() > 0)
				{
					obj[pKey] = Convert::ToFloat(pValue);
					break;
				}

				this->operator[](pKey) = Convert::ToFloat(pValue);
				break;
			}

			return true;
		}
	}

	NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad value ... value is from unknown type ... got '%s'"), pValue);
	return false;
}

bool Net::Json::Object::DeserializeAny(Net::ViewString& key, Net::ViewString& value, Vector<Net::ViewString*>& object_chain, bool m_prepareString)
{
	if (!key.valid() || !value.valid())
	{
		/*
		* some internal error
		*/
		NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Internal parsing error ... view string is not valid"));
		this->Destroy();
		return false;
	}

	Net::Json::Type m_type = Net::Json::Type::NULLVALUE;

	// with object chain
	Net::Json::BasicValueRead obj(nullptr);
	if (object_chain.size() > 0)
	{
		obj = { this->operator[](*object_chain[0]) };
		for (size_t i = 1; i < object_chain.size(); ++i)
		{
			auto vs = *object_chain[i];
			obj = obj[vs];
		}
	}

	/*
	* check for object
	*/
	{
		if (value[value.start()] == '{')
		{
			m_type = Net::Json::Type::OBJECT;
		}

		if (value[value.end() - 1] == '}'
			&& m_type != Net::Json::Type::OBJECT)
		{
			// we got an ending curly for an object, but missing the starting curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad object ... got an ending curly for an object, but missing the starting curly ... '%s'"), value.get());
			return false;
		}
		else if (value[value.end() - 1] != '}'
			&& m_type == Net::Json::Type::OBJECT)
		{
			// we got a starting curly for an object, but missing the ending curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad object ... got a starting curly for an object, but missing the ending curly ... '%s'"), value.get());
			return false;
		}
		else if (m_type == Net::Json::Type::OBJECT)
		{
			// object seem to be fine, now call it's deserializer
			object_chain.push_back(&key);
			{
				// need this line otherwise empty objects do not work
				if (object_chain.size() > 0)
				{
					obj[key] = (BasicObject)Net::Json::Object(true);
				}
				else
				{
					this->operator[](key) = (BasicObject)Net::Json::Object(true);
				}

				if (!this->Deserialize(value, object_chain, m_prepareString))
					return false;
			}
			object_chain.erase(object_chain.size() - 1);

			return true;
		}
	}

	/*
	* check for array
	*/
	{
		if (value[value.start()] == '[')
		{
			m_type = Net::Json::Type::ARRAY;
		}

		if (value[value.end() - 1] == ']'
			&& m_type != Net::Json::Type::ARRAY)
		{
			// we got an ending curly for an array, but missing the starting curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad array ... got an ending curly for an array, but missing the starting curly ... '%s'"), value.get());
			return false;
		}
		else if (value[value.end() - 1] != ']'
			&& m_type == Net::Json::Type::ARRAY)
		{
			// we got a starting curly for an array, but missing the ending curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad array ... got a starting curly for an array, but missing the ending curly ... '%s'"), value.get());
			return false;
		}
		else if (m_type == Net::Json::Type::ARRAY)
		{
			// array seem to be fine, now call it's deserializer
			{
				Net::Json::Array arr(true);
				if (!arr.Deserialize(value, m_prepareString))
				{
					// @todo: add logging
					return false;
				}

				if (object_chain.size() > 0)
				{
					obj[key] = (BasicArray)arr;
				}
				else
				{
					this->operator[](key) = (BasicArray)arr;
				}
			}

			return true;
		}
	}

	/*
	* check for string
	*/
	{
		if (value[value.start()] == '"')
		{
			m_type = Net::Json::Type::STRING;
		}

		if (value[value.end() - 1] == '"'
			&& m_type != Net::Json::Type::STRING)
		{
			// we got an ending double-qoute for a string, but missing the starting
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad string ... got an ending double-qoute for a string, but missing the starting ... '%s'"), value.get());
			return false;
		}
		else if (value[value.end() - 1] != '"'
			&& m_type == Net::Json::Type::STRING)
		{
			// we got a starting double-qoute for a string, but missing the ending
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad string ... got a starting double-qoute for a string, but missing the ending ... '%s'"), value.get());
			return false;
		}
		else if (m_type == Net::Json::Type::STRING)
		{
			/*
			* walk through the string and make sure there are no non-escaped double-qoutes
			*/
			for (int j = value.start() + 1; j < value.end() - 2; ++j)
			{
				auto ec = value[j];
				if (ec == '"')
				{
					if ((j - 1) < 0
						|| value[j - 1] != '\\')
					{
						NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad string ... string contains double-qoutes that are not escaped ... double-qoutes inside a string must be escaped with '\\'"));
						return false;
					}
				}
			}

			// obtain the string from the json-string without the double-qoutes
			auto vs = value.sub_view(value.start() + 1, value.length() - 1);
			auto decoded_string = Net::Json::DecodeString(vs);
			auto decoded_string_ref = decoded_string.get();

			if (object_chain.size() > 0)
			{
				obj[key] = decoded_string_ref.get();
			}
			else
			{
				this->operator[](key) = decoded_string_ref.get();
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
				obj[key] = Convert::ToBoolean(value);
				return true;
			}

			this->operator[](key) = Convert::ToBoolean(value);
			return true;
		}
	}

	/*
	* check for null value
	*/
	{
		if (!memcmp(&value.get()[value.start()], CSTRING("null"), value.size()))
		{
			m_type = Net::Json::Type::NULLVALUE;

			if (object_chain.size() > 0)
			{
				obj[key] = Net::Json::NullValue();
				return true;
			}

			this->operator[](key) = Net::Json::NullValue();
			return true;
		}
	}

	/*
	* check for number
	*/
	{
		m_type = Net::Json::Type::INTEGER;

		constexpr const char m_NumericPattern[] = "0123456789";
		for (size_t i = value.start(), dot = 0; i < value.end(); ++i)
		{
			auto c = value[i];

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
					break;
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
					obj[key] = Convert::ToInt32(value);
					break;
				}

				this->operator[](key) = Convert::ToInt32(value);
				break;

			case Net::Json::Type::DOUBLE:
				if (object_chain.size() > 0)
				{
					obj[key] = Convert::ToDouble(value);
					break;
				}

				this->operator[](key) = Convert::ToDouble(value);
				break;

			case Net::Json::Type::FLOAT:
				if (object_chain.size() > 0)
				{
					obj[key] = Convert::ToFloat(value);
					break;
				}

				this->operator[](key) = Convert::ToFloat(value);
				break;
			}

			return true;
		}
	}

	NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad value ... value is from unknown type ... got '%s'"), &value.get()[value.start()]);
	return false;
}

/*
* This function will try to serialize it
* on any error it will recursivly return and m_valid will be set to false to handle the closing
*/
bool Net::Json::Object::TrySerialize(SerializeType type, SerializeT& st, size_t iterations)
{
	if (!st.m_reserved)
	{
		size_t m_size = CalcLengthForSerialize();
		for (size_t i = 0; i < value.size(); ++i)
		{
			auto tmp = (BasicValue<void*>*)value[i];
			switch (tmp->GetType())
			{
			case Type::OBJECT:
				m_size += tmp->as_object()->CalcLengthForSerialize();
				break;

			case Type::ARRAY:
				m_size += tmp->as_array()->CalcLengthForSerialize();
				break;
			}
		}

		st.m_buffer.reserve(m_size * 2);
		st.m_reserved = !st.m_reserved;
	}

	st.m_buffer += "{";

	if (value.size() != 0 && type == SerializeType::FORMATTED)
		st.m_buffer += '\n';

	for (size_t i = 0; i < value.size(); ++i)
	{
		if (type == SerializeType::FORMATTED)
		{
			for (size_t i = 0; i < iterations; ++i)
				st.m_buffer += '\t';
		}

		auto tmp = (BasicValue<void*>*)value[i];
		Net::String encodedKey(reinterpret_cast<const char*>(tmp->Key()));
		Net::Json::EncodeString(encodedKey);
		
		// append key
		st.m_buffer += '"';
		st.m_buffer += encodedKey;
		st.m_buffer += '"';
		if (type == SerializeType::FORMATTED)
			st.m_buffer += " : ";
		else
			st.m_buffer += ":";

		switch (tmp->GetType())
		{
		case Type::OBJECT:
		{
			if (!tmp->as_object()->TrySerialize(type, st, iterations + 1))
			{
				st.m_buffer = Net::String();
				return false;
			}
			break;
		}

		case Type::ARRAY:
		{
			if (!tmp->as_array()->TrySerialize(type, st, iterations + 1))
			{
				st.m_buffer = Net::String();
				return false;
			}
			break;
		}

		case Type::NULLVALUE:
			st.m_buffer += "null";
			break;

		case Type::STRING:
		{
			Net::String enc(reinterpret_cast<const char*>(tmp->as_string()));
			Net::Json::EncodeString(enc);
			st.m_buffer += '"';
			st.m_buffer += enc;
			st.m_buffer += '"';
			break;
		}

		case Type::INTEGER:
			st.m_buffer.append("%i", tmp->as_int());
			break;

		case Type::BOOLEAN:
			st.m_buffer.append("%s", tmp->as_boolean() ? "true" : "false");
			break;

		case Type::FLOAT:
			st.m_buffer.append("%f", tmp->as_float());
			break;

		case Type::DOUBLE:
			st.m_buffer.append("%lf", tmp->as_double());
			break;

		default:
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] - Unable to serialize object => invalid type"));

			st.m_buffer = Net::String();
			return false;
		}

		if (i != value.size() - 1)
		{
			st.m_buffer += ',';

			if (type == SerializeType::FORMATTED)
				st.m_buffer += '\n';
		}
	}

	if (value.size() != 0 && type == SerializeType::FORMATTED)
	{
		st.m_buffer += '\n';

		for (size_t i = 0; i < iterations - 1; ++i)
			st.m_buffer += '\t';
	}

	st.m_buffer += "}";
	return true;
}

/*
*	This method does do:
*		- it checks if the json string starts of with '{' and ends with '}'
*		- it reads the single elements inside of the object and checks for its split that determinate the key and the value
*		- it reads the entire object or array and stores it as value (use recursive for further analyses)
*		- it realises if any splitters are missing or if too many have been placed
*			- it does not do this very accurate but good enough
*		- it checks if the key is a string
*		- it checks if the key contains any double-qoutes and if it does then it checks if they are escaped
*/
bool Net::Json::Object::Deserialize(Net::String& json, Vector<char*>& object_chain, bool m_prepareString)
{
	/*
	* Prepare the json string before parsing
	*/
	if (!m_prepareString)
	{
		if (!Net::Json::PrepareString(json))
		{
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Unexpected error ... failed to prepare json string ... got '%s'"), json.get().get());
			return false;
		}

		m_prepareString = !m_prepareString;
	}

	/*
	* use view string method rather default way
	* because it requires too many heap allocations
	*/
	auto vs = json.view_string();
	return this->Deserialize(vs, m_prepareString);
}

bool Net::Json::Object::Deserialize(Net::ViewString& json, Vector<Net::ViewString*>& object_chain, bool m_prepareString)
{
	/*
	* Prepare the json string before parsing
	*/
	if (!m_prepareString)
	{
		if (!Net::Json::PrepareString(json))
		{
			NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Unexpected error ... failed to prepare json string ... got '%s'"), json.get());
			return false;
		}

		m_prepareString = !m_prepareString;
	}

	/*
	* Since we are passing the json string into an object deserializer
	* we will check for the syntax that does define an object in the json language at the beginning and end
	* if not then abort the parsing
	*/
	if (json[json.start()] != '{')
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Unexpected character in the beginning of the string ... got '%c' ... expected '{'"), json[json.start()]);
		this->Destroy();
		return false;
	}

	if (json[json.end() - 1] != '}')
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Unexpected character in the ending of the string ... got '%c' ... expected '}'"), json[json.end() - 1]);
		this->Destroy();
		return false;
	}

	uint8_t flag = 0;
	size_t v = 0; // begin for substr
	size_t arr_count = 0;
	size_t obj_count = 0;

	Net::ViewString key = {};

	/*
	* So an object is seperated in its key and value pair
	* first scan for the key and detect its syntax that deals as the seperator (':')
	*/
	flag |= (int)EDeserializeFlag::FLAG_READING_ELEMENT;
	v = json.start();

	/*
	* Since we did check the start and end to match its syntax that does define an object
	* so we can skip the first and last character for the further parsing
	*/
	for (size_t i = json.start() + 1; i < json.end() - 1; ++i)
	{
		auto c = json[i];

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
					this->Destroy();
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
					this->Destroy();
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
					this->Destroy();
					return false;
				}

				/*
				* the last key is obviously the last position at which we set the flag to reading the key
				* now we have determinated the key, but the value is not determinated yet
				* walk forward till we reach the syntax for the seperator for an element or till we reach the end of file
				*/
				key = json.sub_view(v + 1, i - v - 1);
				if (!key.valid())
				{
					/*
					* some internal error
					*/
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Internal parsing error ... view string is not valid"));
					this->Destroy();
					return false;
				}

				/*
				* a key in the json language must be a string
				* and should be unique in each element
				*/
				if (key[key.start()] != '"')
				{
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad key ... key is not a string ... it must start with double quotes ... instead got '%c'"), key[key.start()]);
					this->Destroy();
					return false;
				}

				if (key[key.end() - 1] != '"')
				{
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad key ... key is not a string ... it must end with double quotes ... instead got '%c'"), key[key.end() - 1]);
					this->Destroy();
					return false;
				}

				/*
				* walk through the key and make sure there are no non-escaped double-qoutes
				*/
				for (int j = key.start() + 1; j < key.end() - 2; ++j)
				{
					auto ec = key[j];
					if (ec == '"')
					{
						if ((j - 1) < 0
							|| key[j - 1] != '\\')
						{
							NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad key ... key contains double-qoutes that are not escaped ... double-qoutes inside a key must be escaped with '\\'"));
							this->Destroy();
							return false;
						}
					}
				}

				// obtain the string from the json-string without the double-qoutes
				key = key.sub_view(key.start() + 1, key.length() - 1);
				if (!key.valid())
				{
					/*
					* some internal error
					*/
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Internal parsing error ... view string is not valid"));
					this->Destroy();
					return false;
				}

				flag |= (int)EDeserializeFlag::FLAG_READING_OBJECT_VALUE;
				v = i;
			}

			/*
			* check for the syntax that seperates elements inside an object
			* or check if we have reached eof (end of file)
			*/
			if (c == ',' || (i == json.end() - 2))
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
					this->Destroy();
					return false;
				}

				if ((flag & (int)EDeserializeFlag::FLAG_READING_OBJECT) && obj_count > 0)
				{
					// we got another seperator for an element but missing the end curly for an object
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad syntax ... expected '}' ... got ',' OR 'EOF'"));
					this->Destroy();
					return false;
				}

				if ((flag & (int)EDeserializeFlag::FLAG_READING_ARRAY) && arr_count > 0)
				{
					// we got another seperator for an element but missing the end curly for an array
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad syntax ... expected ']' ... got ',' OR 'EOF'"));
					this->Destroy();
					return false;
				}

				Net::ViewString value = (i == json.end() - 2) ? json.sub_view(v + 1, i - v) : json.sub_view(v + 1, i - v - 1);
				if (!value.valid())
				{
					/*
					* some internal error
					*/
					NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Internal parsing error ... view string is not valid"));
					this->Destroy();
					return false;
				}

				/*
				* pass the value to the DeserializeAny method
				* that method will take any value and further analyse its type
				* it will use recursive to deserialize further object's or array's
				*/
				if (!DeserializeAny(key, value, object_chain, m_prepareString))
				{
					// no need to display error message, the method will do it
					this->Destroy();
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
		this->Destroy();
		return false;
	}

	if (obj_count > 0)
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Object] -> Bad syntax ... expected an object ending curly '}'"));
		this->Destroy();
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
	if (!bSharedMemory) this->Destroy();
}

void Net::Json::Array::Destroy()
{
	this->value.clear();
}

template <typename T>
bool Net::Json::Array::emplace_back(T value, Type type)
{
	BasicValue<T>* heap = ALLOC<BasicValue<T>>();
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
	char* ptr = ALLOC<char>(len + 1);
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

size_t Net::Json::Array::CalcLengthForSerialize()
{
	size_t m_size = 0;
	for (size_t i = 0; i < value.size(); ++i)
	{
		auto tmp = (BasicValue<void*>*)value[i];
		if (tmp->GetType() == Type::NULLVALUE)
		{
			m_size += NET_JSON_ARR_LEN("null");
		}
		else if (tmp->GetType() == Type::STRING)
		{
			auto len = strlen(tmp->as_string());
			m_size += len;
		}
		else if (tmp->GetType() == Type::INTEGER)
		{
			if (tmp->as_int() == 0) m_size += 1;
			else m_size += floor(log10(abs(tmp->as_int()))) + 1;
		}
		else if (tmp->GetType() == Type::FLOAT)
		{
			int digits = 0;
			long cnum = tmp->as_float();
			while (cnum > 0) //count no of digits before floating point
			{
				digits++;
				cnum = cnum / 10;
			}
			if (tmp->as_float() == 0)
				digits = 1;

			double no_float;
			no_float = tmp->as_float() * (pow(10, (8 - digits)));
			long long int total = (long long int)no_float;
			int no_of_digits, extrazeroes = 0;
			for (int i = 0; i < 8; i++)
			{
				int dig;
				dig = total % 10;
				total = total / 10;
				if (dig != 0)
					break;
				else
					extrazeroes++;
			}
			no_of_digits = 8 - extrazeroes;
			m_size += no_of_digits;
		}
		else if (tmp->GetType() == Type::DOUBLE)
		{
			int digits = 0;
			long cnum = tmp->as_double();
			while (cnum > 0) //count no of digits before floating point
			{
				digits++;
				cnum = cnum / 10;
			}
			if (tmp->as_double() == 0)
				digits = 1;

			double no_float;
			no_float = tmp->as_double() * (pow(10, (16 - digits)));
			long long int total = (long long int)no_float;
			int no_of_digits, extrazeroes = 0;
			for (int i = 0; i < 16; i++)
			{
				int dig;
				dig = total % 10;
				total = total / 10;
				if (dig != 0)
					break;
				else
					extrazeroes++;
			}
			no_of_digits = 16 - extrazeroes;
			m_size += no_of_digits;
		}
		else if (tmp->GetType() == Type::BOOLEAN)
		{
			if (tmp->as_boolean())
			{
				m_size += NET_JSON_ARR_LEN("true");
			}
			else
			{
				m_size += NET_JSON_ARR_LEN("false");
			}
		}
		else if (tmp->GetType() == Type::OBJECT)
		{
			m_size += tmp->as_object()->CalcLengthForSerialize();
		}
		else if (tmp->GetType() == Type::ARRAY)
		{
			m_size += tmp->as_array()->CalcLengthForSerialize();
		}
	}
	return m_size;
}

Net::String Net::Json::Array::Serialize(SerializeType type)
{
	Net::Json::SerializeT st;
	st.m_reserved = false;
	if (!this->TrySerialize(type, st))
	{
		return {};
	}
	st.m_buffer.finalize();
	return st.m_buffer;
}

Net::String Net::Json::Array::Stringify(SerializeType type)
{
	return this->Serialize(type);
}

bool Net::Json::Array::Deserialize(Net::String json)
{
	/*
	* use view string method rather default way
	* because it requires too many heap allocations
	*/
	auto vs = json.view_string();
	return this->Deserialize(vs, false);
}

bool Net::Json::Array::Deserialize(Net::ViewString& json)
{
	return this->Deserialize(json, false);
}

bool Net::Json::Array::DeserializeAny(Net::String& value, bool m_prepareString)
{
	Net::Json::Type m_type = Net::Json::Type::NULLVALUE;

	auto valueRef = value.get();
	auto pValue = valueRef.get();

	/*
	* check for object
	*/
	{
		if (value[0] == '{')
		{
			m_type = Net::Json::Type::OBJECT;
		}

		if (value[value.length()] == '}'
			&& m_type != Net::Json::Type::OBJECT)
		{
			// we got an ending curly for an object, but missing the starting curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad object ... got an ending curly for an object, but missing the starting curly ... '%s'"), pValue);
			return false;
		}
		else if (value[value.length()] != '}'
			&& m_type == Net::Json::Type::OBJECT)
		{
			// we got a starting curly for an object, but missing the ending curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad object ... got a starting curly for an object, but missing the ending curly ... '%s'"), pValue);
			return false;
		}
		else if (m_type == Net::Json::Type::OBJECT)
		{
			// object seem to be fine, now call it's deserializer
			Net::Json::Object obj(true);
			if (!obj.Deserialize(value, m_prepareString))
			{
				// @todo: add logging
				return false;
			}
			return this->push(obj);
		}
	}

	/*
	* check for array
	*/
	{
		if (value[0] == '[')
		{
			m_type = Net::Json::Type::ARRAY;
		}

		if (value[value.length()] == ']'
			&& m_type != Net::Json::Type::ARRAY)
		{
			// we got an ending curly for an array, but missing the starting curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad array ... got an ending curly for an array, but missing the starting curly ... '%s'"), pValue);
			return false;
		}
		else if (value[value.length()] != ']'
			&& m_type == Net::Json::Type::ARRAY)
		{
			// we got a starting curly for an array, but missing the ending curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad array ... got a starting curly for an array, but missing the ending curly ... '%s'"), pValue);
			return false;
		}
		else if (m_type == Net::Json::Type::ARRAY)
		{
			// array seem to be fine, now call it's deserializer
			Net::Json::Array arr(true);
			if (!arr.Deserialize(value, m_prepareString))
			{
				// @todo: add logging
				return false;
			}
			return this->push(arr);
		}
	}

	/*
	* check for string
	*/
	{
		if (value[0] == '"')
		{
			m_type = Net::Json::Type::STRING;
		}

		if (value[value.length()] == '"'
			&& m_type != Net::Json::Type::STRING)
		{
			// we got an ending double-qoute for a string, but missing the starting
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad string ... got an ending double-qoute for a string, but missing the starting ... '%s'"), pValue);
			return false;
		}
		else if (value[value.length()] != '"'
			&& m_type == Net::Json::Type::STRING)
		{
			// we got a starting double-qoute for a string, but missing the ending
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad string ... got a starting double-qoute for a string, but missing the ending ... '%s'"), pValue);
			return false;
		}
		else if (m_type == Net::Json::Type::STRING)
		{
			/*
			* walk through the string and make sure there are no non-escaped double-qoutes
			*/
			for (int j = 1; j < value.length() - 1; ++j)
			{
				auto ec = value[j];
				if (ec == '"')
				{
					if ((j - 1) < 0
						|| value[j - 1] != '\\')
					{
						NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad string ... string contains double-qoutes that are not escaped ... double-qoutes inside a string must be escaped with '\\'"));
						return false;
					}
				}
			}

			// obtain the string from the json-string without the double-qoutes
			Net::String str(value.substr(1, value.length() - 1));
			Net::Json::DecodeString(str);
			return this->push(str.get().get());
		}
	}

	/*
	* check for boolean
	*/
	{
		if (Convert::is_boolean(pValue))
		{
			m_type = Net::Json::Type::BOOLEAN;
			return this->push(Convert::ToBoolean(pValue));
		}
	}

	/*
	* check for null value
	*/
	{
		if (!memcmp(pValue, CSTRING("null"), 4))
		{
			m_type = Net::Json::Type::NULLVALUE;
			return this->push(Net::Json::NullValue());
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
			auto c = value[i];

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
					break;
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
				if (Convert::is_double(pValue))
				{
					m_type = Net::Json::Type::DOUBLE;
				}
				else if (Convert::is_float(pValue))
				{
					m_type = Net::Json::Type::FLOAT;
				}
			}

			switch (m_type)
			{
			case Net::Json::Type::INTEGER:
				return this->push(Convert::ToInt32(pValue));

			case Net::Json::Type::DOUBLE:
				return this->push(Convert::ToDouble(pValue));

			case Net::Json::Type::FLOAT:
				return this->push(Convert::ToFloat(pValue));

			default:
				break;
			}
		}
	}

	NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad value ... value is from unknown type ... got '%s'"), pValue);
	return false;
}

bool Net::Json::Array::DeserializeAny(Net::ViewString& vs, bool m_prepareString)
{
	if (!vs.valid())
	{
		/*
		* some internal error
		*/
		NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Internal parsing error ... view string is not valid"));
		this->Destroy();
		return false;
	}

	Net::Json::Type m_type = Net::Json::Type::NULLVALUE;

	/*
	* check for object
	*/
	{
		if (vs[vs.start()] == '{')
		{
			m_type = Net::Json::Type::OBJECT;
		}

		if (vs[vs.end() - 1] == '}'
			&& m_type != Net::Json::Type::OBJECT)
		{
			// we got an ending curly for an object, but missing the starting curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad object ... got an ending curly for an object, but missing the starting curly ... '%s'"), vs.get());
			return false;
		}
		else if (vs[vs.end() - 1] != '}'
			&& m_type == Net::Json::Type::OBJECT)
		{
			// we got a starting curly for an object, but missing the ending curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad object ... got a starting curly for an object, but missing the ending curly ... '%s'"), vs.get());
			return false;
		}
		else if (m_type == Net::Json::Type::OBJECT)
		{
			// object seem to be fine, now call it's deserializer
			Net::Json::Object obj(true);
			if (!obj.Deserialize(vs, m_prepareString))
			{
				// @todo: add logging
				return false;
			}
			return this->push(obj);
		}
	}

	/*
	* check for array
	*/
	{
		if (vs[vs.start()] == '[')
		{
			m_type = Net::Json::Type::ARRAY;
		}

		if (vs[vs.end() - 1] == ']'
			&& m_type != Net::Json::Type::ARRAY)
		{
			// we got an ending curly for an array, but missing the starting curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad array ... got an ending curly for an array, but missing the starting curly ... '%s'"), vs.get());
			return false;
		}
		else if (vs[vs.end() - 1] != ']'
			&& m_type == Net::Json::Type::ARRAY)
		{
			// we got a starting curly for an array, but missing the ending curly
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad array ... got a starting curly for an array, but missing the ending curly ... '%s'"), vs.get());
			return false;
		}
		else if (m_type == Net::Json::Type::ARRAY)
		{
			// array seem to be fine, now call it's deserializer
			Net::Json::Array arr(true);
			if (!arr.Deserialize(vs, m_prepareString))
			{
				// @todo: add logging
				return false;
			}
			return this->push(arr);
		}
	}

	/*
	* check for string
	*/
	{
		if (vs[vs.start()] == '"')
		{
			m_type = Net::Json::Type::STRING;
		}

		if (vs[vs.end() - 1] == '"'
			&& m_type != Net::Json::Type::STRING)
		{
			// we got an ending double-qoute for a string, but missing the starting
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad string ... got an ending double-qoute for a string, but missing the starting ... '%s'"), vs.get());
			return false;
		}
		else if (vs[vs.end() - 1] != '"'
			&& m_type == Net::Json::Type::STRING)
		{
			// we got a starting double-qoute for a string, but missing the ending
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad string ... got a starting double-qoute for a string, but missing the ending ... '%s'"), vs.get());
			return false;
		}
		else if (m_type == Net::Json::Type::STRING)
		{
			/*
			* walk through the string and make sure there are no non-escaped double-qoutes
			*/
			for (int j = vs.start() + 1; j < vs.end() - 2; ++j)
			{
				auto ec = vs[j];
				if (ec == '"')
				{
					if ((j - 1) < 0
						|| vs[j - 1] != '\\')
					{
						NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad string ... string contains double-qoutes that are not escaped ... double-qoutes inside a string must be escaped with '\\'"));
						return false;
					}
				}
			}

			// obtain the string from the json-string without the double-qoutes
			auto svs = vs.sub_view(vs.start() + 1, vs.length() - 1);
			auto decoded_string = Net::Json::DecodeString(svs);
			auto decoded_string_ref = decoded_string.get();
			return this->push(decoded_string_ref.get());
		}
	}

	/*
	* check for boolean
	*/
	{
		if (Convert::is_boolean(vs))
		{
			m_type = Net::Json::Type::BOOLEAN;
			return this->push(Convert::ToBoolean(vs));
		}
	}

	/*
	* check for null value
	*/
	{
		if (!memcmp(&vs.get()[vs.start()], CSTRING("null"), vs.size()))
		{
			m_type = Net::Json::Type::NULLVALUE;
			return this->push(Net::Json::NullValue());
		}
	}

	/*
	* check for number
	*/
	{
		m_type = Net::Json::Type::INTEGER;

		constexpr const char m_NumericPattern[] = "0123456789";
		for (size_t i = vs.start(), dot = 0; i < vs.end(); ++i)
		{
			auto c = vs[i];

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
					break;
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
				if (Convert::is_double(vs))
				{
					m_type = Net::Json::Type::DOUBLE;
				}
				else if (Convert::is_float(vs))
				{
					m_type = Net::Json::Type::FLOAT;
				}
			}

			switch (m_type)
			{
			case Net::Json::Type::INTEGER:
				return this->push(Convert::ToInt32(vs));

			case Net::Json::Type::DOUBLE:
				return this->push(Convert::ToDouble(vs));

			case Net::Json::Type::FLOAT:
				return this->push(Convert::ToFloat(vs));

			default:
				break;
			}
		}
	}

	NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad value ... value is from unknown type ... got '%s'"), &vs.get()[vs.start()]);
	return false;
}

/*
* This function will try to serialize it
* on any error it will recursivly return and m_valid will be set to false to handle the closing
*/
bool Net::Json::Array::TrySerialize(SerializeType type, SerializeT& st, size_t iterations)
{
	if (!st.m_reserved)
	{
		size_t m_size = CalcLengthForSerialize();
		for (size_t i = 0; i < value.size(); ++i)
		{
			auto tmp = (BasicValue<void*>*)value[i];
			switch (tmp->GetType())
			{
			case Type::OBJECT:
				m_size += tmp->as_object()->CalcLengthForSerialize();
				break;

			case Type::ARRAY:
				m_size += tmp->as_array()->CalcLengthForSerialize();
				break;
			}
		}

		st.m_buffer.reserve(m_size * 2);
		st.m_reserved = !st.m_reserved;
	}

	st.m_buffer += '[';
	for (size_t i = 0; i < value.size(); ++i)
	{
		auto tmp = (BasicValue<void*>*)value[i];
		switch (tmp->GetType())
		{
		case Type::OBJECT:
		{
			if (!tmp->as_object()->TrySerialize(type, st, iterations + 1))
			{
				st.m_buffer = Net::String();
				return false;
			}
			break;
		}

		case Type::ARRAY:
		{
			if (!tmp->as_array()->TrySerialize(type, st, iterations + 1))
			{
				st.m_buffer = Net::String();
				return false;
			}
			break;
		}

		case Type::NULLVALUE:
			st.m_buffer += "null";
			break;

		case Type::STRING:
		{
			Net::String enc((const char*)tmp->as_string());
			Net::Json::EncodeString(enc);
			st.m_buffer += '"';
			st.m_buffer += enc;
			st.m_buffer += '"';
			break;
		}

		case Type::INTEGER:
			st.m_buffer.append("%i", tmp->as_int());
			break;

		case Type::BOOLEAN:
			st.m_buffer.append("%s", tmp->as_boolean() ? "true" : "false");
			break;

		case Type::FLOAT:
			st.m_buffer.append("%f", tmp->as_float());
			break;

		case Type::DOUBLE:
			st.m_buffer.append("%lf", tmp->as_double());
			break;

		default:
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] - Unable to serialize array => invalid type"));

			st.m_buffer = Net::String();
			return false;
		}

		if (i != value.size() - 1) 	st.m_buffer += ',';
	}
	st.m_buffer += ']';
	return true;
}

/*
* How it is working in its current state
*	- reading object and parsing it using the object deserializer
*	- reading array and parsing it using recursive method
*	- reading string
*	- reading anything
*		- converting anything into int, float, double, boolean and null value
*/
bool Net::Json::Array::Deserialize(Net::String& json, bool m_prepareString)
{
	/*
	* Prepare the json string before parsing
	*/
	if (!m_prepareString)
	{
		if (!Net::Json::PrepareString(json))
		{
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Unexpected error ... failed to prepare json string ... got '%s'"), json.get().get());
			return false;
		}

		m_prepareString = !m_prepareString;
	}

	/*
	* use view string method rather default way
	* because it requires too many heap allocations
	*/
	auto vs = json.view_string();
	return this->Deserialize(vs, m_prepareString);
}

bool Net::Json::Array::Deserialize(Net::ViewString& json, bool m_prepareString)
{
	/*
	* Prepare the json string before parsing
	*/
	if (!m_prepareString)
	{
		if (!Net::Json::PrepareString(json))
		{
			NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Unexpected error ... failed to prepare json string ... got '%s'"), json.get());
			return false;
		}

		m_prepareString = !m_prepareString;
	}

	/*
	* Since we are passing the json string into an array deserializer
	* we will check for the syntax that does define an array in the json language at the beginning and end
	* if not then abort the parsing
	*/
	if (json[json.start()] != '[')
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Unexpected character in the beginning of the string ... got '%c' ... expected '['"), json[json.start()]);
		this->Destroy();
		return false;
	}

	if (json[json.end() - 1] != ']')
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Unexpected character in the ending of the string ... got '%c' ... expected ']'"), json[json.end() - 1]);
		this->Destroy();
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
	v = json.start();

	for (size_t i = json.start() + 1; i < json.end() - 1; ++i)
	{
		auto c = json[i];

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
					this->Destroy();
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
					NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad syntax ... expected '[' ... got ']'"));
					this->Destroy();
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
			* check for the syntax that splits the elements in an array
			* or check if we have reached eof (end of file)
			*	will happen if there is only one element
			*/
			if (c == ',' || (i == json.end() - 2))
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

				Net::ViewString element = (i == json.end() - 2) ? json.sub_view(v + 1, i - v) : json.sub_view(v + 1, i - v - 1);
				if (!element.valid())
				{
					/*
					* some internal error
					*/
					NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Internal parsing error ... view string is not valid"));
					this->Destroy();
					return false;
				}

				/*
				* pass the value to the DeserializeAny method
				* that method will take any value and further analyse its type
				* it will use recursive to deserialize further object's or array's
				*/
				if (!DeserializeAny(element, m_prepareString))
				{
					// no need to display error message, the method will do it
					this->Destroy();
					return false;
				}

				v = i;
			}
		}
	}

	if (arr_count > 0)
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad syntax ... expected an array ending curly ']'"));
		this->Destroy();
		return false;
	}

	if (obj_count > 0)
	{
		NET_LOG_ERROR(CSTRING("[Net::Json::Array] -> Bad syntax ... expected an object ending curly '}'"));
		this->Destroy();
		return false;
	}

	return true;
}

bool Net::Json::Array::Parse(Net::String json)
{
	return this->Deserialize(json);
}

bool Net::Json::Array::Parse(Net::ViewString& json)
{
	return this->Deserialize(json);
}

Net::Json::Document::Document()
{
	this->Init();
}

Net::Json::Document::Document(Net::Json::Document&& m_doc) NOEXPECT
{
	/*
	* move the document into new instance
	*/
	this->m_type = m_doc.m_type;
	this->root_obj = m_doc.root_obj;
	this->root_array = m_doc.root_array;
	this->m_free_root_obj = m_doc.m_free_root_obj;
	this->m_free_root_array = m_doc.m_free_root_array;

	/*
	* m_doc lost it's reference
	* do not free objects on heap
	* they are still in use
	*/
	m_doc.m_free_root_obj = false;
	m_doc.m_free_root_array = false;
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
	if (this->m_free_root_obj) this->root_obj.Destroy();
	if (this->m_free_root_array) this->root_array.Destroy();
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
	/*
	* use view string method rather default way
	* because it requires too many heap allocations
	*/
	auto vs = json.view_string();
	return this->Deserialize(vs);
}

bool Net::Json::Document::Deserialize(Net::ViewString& json)
{
	/* re-init */
	this->Clear();
	this->Init();

	if (json[0] == '{' && json[json.length()] == '}')
	{
		/* is object */
		this->m_type = Net::Json::Type::OBJECT;
		return this->root_obj.Deserialize(json);
	}
	else if (json[0] == '[' && json[json.length()] == ']')
	{
		this->m_type = Net::Json::Type::ARRAY;
		return this->root_array.Deserialize(json);
	}

	NET_LOG_ERROR(CSTRING("[Net::Json::Document] - Unable to deserialize document => neither object nor array"));
	return false;
}

bool Net::Json::Document::Parse(Net::String json)
{
	return this->Deserialize(json);
}

bool Net::Json::Document::Parse(Net::ViewString& json)
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
