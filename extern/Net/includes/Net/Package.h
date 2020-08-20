#pragma once
#define RPackage Package&

#define JOIN_PACKAGE(PKGOLD, PKGNEW) \
	Package PKGNEW; \
	PKGNEW.SetPackage(PKGOLD.GetPackage());

#include <Net/Net.h>
#include <Net/NativePackages.h>

#include <assets/assets.h>

#include <JSON/document.h>
#include <JSON/stringbuffer.h>
#include <JSON/writer.h>

template<typename TYPE>
BEGIN_CLASS(Package_t)
bool _valid;
char _name[256];
TYPE _value;

CLASS_PUBLIC
Package_t(bool _valid, const char* _name, TYPE _value)
{
	this->_valid = _valid;
	strcpy_s(this->_name, _name);
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
END_CLASS

BEGIN_CLASS(Package_t_Object)
bool _valid;
char _name[256];
rapidjson::Value _value;

CLASS_PUBLIC
Package_t_Object(bool _valid, const char* _name, rapidjson::Value _value)
{
	this->_valid = _valid;
	strcpy_s(this->_name, _name);
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
	return _value.GetObjectA();
}
END_CLASS

BEGIN_CLASS(Package_t_Array)
bool _valid;
char _name[256];
rapidjson::Value _value;

CLASS_PUBLIC
Package_t_Array(bool _valid, const char* _name, rapidjson::Value _value)
{
	this->_valid = _valid;
	strcpy_s(this->_name, _name);
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
END_CLASS

BEGIN_NAMESPACE(Net)
BEGIN_CLASS(Package)
rapidjson::Document pkg;

CLASS_PUBLIC
CLASS_CONSTRUCTUR(Package);

template<typename Type>
void Append(const char* Key, const Type Value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		LOG_DEBUG("[JSON][PACKAGE] - Key already exists in JSON Object");
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	rapidjson::Value value;
	value.Set<Type>(Value);

	if (!value.IsNull())
		pkg.AddMember(key, value, pkg.GetAllocator());
}

template<typename Type>
void Rewrite(const char* Key, const Type Value)
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG("[JSON][PACKAGE] - Package is empty");
		return;
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG("[JSON][PACKAGE] - Package is empty");
		return;
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG("[JSON][PACKAGE] - Could not find Key");
		return;
	}

	const auto member = pkg.FindMember(Key);
	member->value.Set<Type>(Value);
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

rapidjson::Document& GetPackage();
std::string StringifyPackage() const;
Package_t<const char*> String(const char*) const;
Package_t<int> Int(const char*) const;
Package_t<double> Double(const char*) const;
Package_t<float> Float(const char*) const;
Package_t<__int64> Int64(const char*) const;
Package_t<UINT> UINT(const char*) const;
Package_t<UINT64> UINT64(const char*) const;
Package_t<bool> Boolean(const char*) const;
Package_t_Object Object(const char* Key);
Package_t_Array Array(const char*);
END_CLASS
END_NAMESPACE

typedef Net::Package JSON;
typedef Net::Package Json;
typedef Net::Package json;