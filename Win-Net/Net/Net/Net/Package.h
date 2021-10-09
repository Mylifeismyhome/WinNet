#pragma once
#define NET_PACKAGE Package&

#define NET_JOIN_PACKAGE(PKGOLD, PKGNEW) \
	Package PKGNEW; \
	PKGNEW.SetPackage(PKGOLD.GetPackage());

///////////////////////////////////////////////////////////
///																	///
///		  FAST PACKAGE ID IMPLEMENTATION	    ///
///																	///
//////////////////////////////////////////////////////////
#define NET_DEFINE_PACKAGES(name, first, ...) \
NET_NAMESPACE_BEGIN(name) \
enum T##name \
{ \
first = NET_LAST_PACKAGE_ID, \
__VA_ARGS__ \
}; \
NET_NAMESPACE_END

#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>
#include <Net/Net/NativePackages.h>

#include <Net/assets/assets.h>

#include <JSON/document.h>
#include <JSON/stringbuffer.h>
#include <JSON/writer.h>

#include <iostream>
#include <string>

#ifdef BUILD_LINUX
#define GetObjectA GetObject
#endif

NET_DSA_BEGIN

NET_CLASS_BEGIN(Package_RawData_t)
char _key[256];
byte* _data;
size_t _size;
bool _free_after_sent; /* by default this value is set to TRUE */
bool _valid;

NET_CLASS_PUBLIC
NET_CLASS_BEGIN_CONSTRUCTUR(Package_RawData_t)
memset(this->_key, NULL, 256);
this->_data = nullptr;
this->_size = NULL;
this->_free_after_sent = false;
this->_valid = false;
NET_CLASS_END_CONTRUCTION

NET_CLASS_BEGIN_CONSTRUCTUR(Package_RawData_t, const char* name, byte* pointer, const size_t size)
strcpy(this->_key, name);
this->_data = pointer;
this->_size = size;
this->_free_after_sent = true;
this->_valid = true;
NET_CLASS_END_CONTRUCTION

NET_CLASS_BEGIN_CONSTRUCTUR(Package_RawData_t, const char* name, byte* pointer, const size_t size, const bool free_after_sent)
strcpy(this->_key, name);
this->_data = pointer;
this->_size = size;
this->_free_after_sent = free_after_sent;
this->_valid = true;
NET_CLASS_END_CONTRUCTION

bool valid() const
{
	return _valid;
}

byte* value() const
{
	return _data;
}

byte*& value()
{
	return _data;
}

size_t size() const
{
	return _size;
}

size_t& size()
{
	return _size;
}

void set_free(bool free)
{
	_free_after_sent = free;

}

bool do_free() const
{
	return _free_after_sent;
}

const char* key() const
{
	if (_key == nullptr) return CSTRING("");
	return _key;
}

void set(byte* pointer)
{
	if (do_free())  FREE(_data);
	_data = pointer;
}

void free()
{
	if (!this->_valid) return;

	// only free the passed reference if we allow it
	if(do_free()) FREE(this->_data);

	this->_data = nullptr;
	this->_size = NULL;

	this->_valid = false;
}
NET_CLASS_END

template<typename TYPE>
NET_CLASS_BEGIN(Package_t)
bool _valid;
char _name[256];
TYPE _value;

NET_CLASS_PUBLIC
Package_t(const bool _valid, const char* _name, TYPE _value)
{
	this->_valid = _valid;
	strcpy(this->_name, _name);
	this->_value = _value;
}

bool valid() const
{
	return _valid;
}

const char* name() const
{
	return _name;
}

TYPE value() const
{
	return _value;
}
NET_CLASS_END

NET_CLASS_BEGIN(Package_t_Object)
bool _valid;
char _name[256];
rapidjson::Value _value;

NET_CLASS_PUBLIC
Package_t_Object(const bool _valid, const char* _name, rapidjson::Value _value)
{
	this->_valid = _valid;
	strcpy(this->_name, _name);
	this->_value = _value;
}

bool valid() const
{
	return _valid;
}

const char* name() const
{
	return _name;
}

rapidjson::Value::Object value()
{
	return _value.GetObject();
}
NET_CLASS_END

NET_CLASS_BEGIN(Package_t_Array)
bool _valid;
char _name[256];
rapidjson::Value _value;

NET_CLASS_PUBLIC
Package_t_Array(const bool _valid, const char* _name, rapidjson::Value _value)
{
	this->_valid = _valid;
	strcpy(this->_name, _name);
	this->_value = _value;
}

bool valid() const
{
	return _valid;
}

const char* name() const
{
	return _name;
}

rapidjson::Value::Array value()
{
	return _value.GetArray();
}
NET_CLASS_END

NET_NAMESPACE_BEGIN(Net)
NET_CLASS_BEGIN(Package)
rapidjson::Document pkg;
std::vector<Package_RawData_t> rawData;

NET_CLASS_PUBLIC
explicit NET_CLASS_CONSTRUCTUR(Package);
NET_CLASS_DESTRUCTUR(Package);

template<typename Type>
void Append(const char* Key, const Type Value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Key already exists in JSON Object"));
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	rapidjson::Value value;
	value.Set<Type>(Value);

	if (!value.IsNull())
		pkg.AddMember(key, value, pkg.GetAllocator());
}

void AppendRawData(const char* Key, BYTE* data, const size_t size, const bool free_after_sent = true)
{
	for (auto& entry : rawData)
	{
		if (!strcmp(entry.key(), Key))
		{
			if (free_after_sent)
			{
				LOG_ERROR(CSTRING("Duplicated Key, buffer gets automaticly deleted from heap to avoid memory leaks"));
				FREE(data);
				return;
			}

			LOG_ERROR(CSTRING("Duplicated Key, buffer has not been deleted from heap"));
			return;
		}
	}

	rawData.emplace_back(Package_RawData_t(Key, data, size, free_after_sent));
}

template<typename Type>
void Rewrite(const char* Key, const Type Value)
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Package is empty"));
		return;
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Package is empty"));
		return;
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Could not find Key"));
		return;
	}

	const auto member = pkg.FindMember(Key);
	member->value.Set<Type>(Value);
}

void RewriteRawData(const char* Key, BYTE* data)
{
	for (auto entry : rawData)
	{
		if (!strcmp(entry.key(), Key))
		{
			entry.set(data);
			break;
		}
	}
}

bool Parse(const char* data);

void SetPackage(const rapidjson::Document& doc)
{
	this->pkg.CopyFrom(doc, this->pkg.GetAllocator());
}

void SetPackage(const rapidjson::Value& doc)
{
	this->pkg.CopyFrom(doc, this->pkg.GetAllocator());
}

void SetPackage(const rapidjson::Document::Object& obj)
{
	this->pkg.Set(obj);
}

void SetPackage(const rapidjson::Document::ValueIterator& arr)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	arr->Accept(writer);
	this->pkg.Parse(buffer.GetString());
}

void SetRawData(const std::vector<Package_RawData_t>& data)
{
	rawData = data;
}

std::vector<Package_RawData_t>& GetRawData()
{
	return rawData;
}

bool HasRawData() const
{
	return !rawData.empty();
}

size_t GetRawDataFullSize() const
{
	size_t size = NULL;
	for (auto& entry : rawData)
	{
		size += strlen(NET_RAW_DATA_KEY);
		size += 1;
		const auto KeyLengthStr = std::to_string(strlen(entry.key()) + 1);
		size += KeyLengthStr.size();
		size += 1;
		size += strlen(NET_RAW_DATA);
		size += 1;
		const auto rawDataLengthStr = std::to_string(entry.size());
		size += rawDataLengthStr.size();
		size += 1;
		size += strlen(entry.key()) + 1;
		size += entry.size();
	}

	return size;
}

rapidjson::Document& GetPackage();
std::string StringifyPackage() const;
Package_t<const char*> String(const char*) const;
Package_t<int> Int(const char*) const;
Package_t<double> Double(const char*) const;
Package_t<float> Float(const char*) const;
Package_t<int64> Int64(const char*) const;
Package_t<uint> UINT(const char*) const;
Package_t<uint64> UINT64(const char*) const;
Package_t<bool> Boolean(const char*) const;
Package_t_Object Object(const char*);
Package_t_Array Array(const char*);
Package_RawData_t RawData(const char*);
NET_CLASS_END
NET_NAMESPACE_END
NET_DSA_END
