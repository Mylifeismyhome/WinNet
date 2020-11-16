#include <Net/Package.h>

NET_NAMESPACE_BEGIN(Net)
Package::Package()
{
	pkg.SetObject();
	bDoNotDesturct = false;
}

Package::~Package()
{
	if(!bDoNotDesturct)
	{
		for (auto& entry : rawData)
			entry.free();
	}
}

bool Package::Parse(const char* data)
{
	return !this->pkg.Parse(data).HasParseError();
}

rapidjson::Document& Package::GetPackage()
{
	return pkg;
}

std::string Package::StringifyPackage() const
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	pkg.Accept(writer);
	std::string outBuffer = buffer.GetString();
	return outBuffer;
}

Package_t<const char*> Package::String(const char* Key) const
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The array of this package is empty"));
		return { false, Key, "" };
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The object of this package is empty"));
		return { false, Key, "" };
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Key ('%s') is not a part of this package"), Key);
		return { false, Key, "" };
	}

	if (pkg.FindMember(Key)->value.IsNull())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is null"), Key);
		return { false, Key, "" };
	}

	if(!pkg.FindMember(Key)->value.IsString())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not a string"), Key);
		return { false, Key, "" };
	}

	return { true, Key, pkg.FindMember(Key)->value.GetString() };
}

Package_t<int> Package::Int(const char* Key) const
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The array of this package is empty"));
		return { false, Key, 0 };
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The object of this package is empty"));
		return { false, Key, 0 };
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Key ('%s') is not a part of this package"), Key);
		return { false, Key, 0 };
	}

	if (pkg.FindMember(Key)->value.IsNull())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is null"), Key);
		return { false, Key, 0 };
	}

	if (!pkg.FindMember(Key)->value.IsInt())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not an integer"), Key);
		return { false, Key, 0 };
	}

	return { true, Key, pkg.FindMember(Key)->value.GetInt() };
}

Package_t<double> Package::Double(const char* Key) const
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The array of this package is empty"));
		return { false, Key, 0 };
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The object of this package is empty"));
		return { false, Key, 0 };
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Key ('%s') is not a part of this package"), Key);
		return { false, Key, 0 };
	}

	if (pkg.FindMember(Key)->value.IsNull())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is null"), Key);
		return { false, Key, 0 };
	}

	if (!pkg.FindMember(Key)->value.IsDouble())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not a double"), Key);
		return { false, Key, 0 };
	}

	return { true, Key, pkg.FindMember(Key)->value.GetDouble() };
}

Package_t<float> Package::Float(const char* Key) const
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The array of this package is empty"));
		return { false, Key, 0 };
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The object of this package is empty"));
		return { false, Key, 0 };
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Key ('%s') is not a part of this package"), Key);
		return { false, Key, 0 };
	}

	if (pkg.FindMember(Key)->value.IsNull())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is null"), Key);
		return { false, Key, 0 };
	}

	if (!pkg.FindMember(Key)->value.IsFloat())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not a float"), Key);
		return { false, Key, 0 };
	}

	return { true, Key,  pkg.FindMember(Key)->value.GetFloat() };
}

Package_t<__int64> Package::Int64(const char* Key) const
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The array of this package is empty"));
		return { false, Key, 0 };
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The object of this package is empty"));
		return { false, Key, 0 };
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Key ('%s') is not a part of this package"), Key);
		return { false, Key, 0 };
	}

	if (pkg.FindMember(Key)->value.IsNull())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is null"), Key);
		return { false, Key, 0 };
	}

	if (!pkg.FindMember(Key)->value.IsInt64())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not an integer of 64 bits"), Key);
		return { false, Key, 0 };
	}

	return { true, Key, pkg.FindMember(Key)->value.GetInt64() };
}

Package_t<UINT> Package::UINT(const char* Key) const
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The array of this package is empty"));
		return { false, Key, 0 };
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The object of this package is empty"));
		return { false, Key, 0 };
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Key ('%s') is not a part of this package"), Key);
		return { false, Key, 0 };
	}

	if (pkg.FindMember(Key)->value.IsNull())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is null"), Key);
		return { false, Key, 0 };
	}

	if (!pkg.FindMember(Key)->value.IsUint())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not an unsigned integer of 32 bits"), Key);
		return { false, Key, 0 };
	}

	return { true, Key, pkg.FindMember(Key)->value.GetUint() };
}

Package_t<UINT64> Package::UINT64(const char* Key) const
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The array of this package is empty"));
		return { false, Key, 0 };
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The object of this package is empty"));
		return { false, Key, 0 };
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Key ('%s') is not a part of this package"), Key);
		return { false, Key, 0 };
	}

	if (pkg.FindMember(Key)->value.IsNull())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is null"), Key);
		return { false, Key, 0 };
	}

	if (!pkg.FindMember(Key)->value.IsUint64())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not an unsigned integer of 64 bits"), Key);
		return { false, Key, 0 };
	}

	return { true, Key, pkg.FindMember(Key)->value.GetUint64() };
}

Package_t<bool> Package::Boolean(const char* Key) const
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The array of this package is empty"));
		return { false, Key, false };
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The object of this package is empty"));
		return { false, Key, false };
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Key ('%s') is not a part of this package"), Key);
		return { false, Key, false };
	}

	if (pkg.FindMember(Key)->value.IsNull())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is null"), Key);
		return { false, Key, false };
	}

	if (!pkg.FindMember(Key)->value.IsBool())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not a boolean"), Key);
		return { false, Key, false };
	}

	return { true, Key, pkg.FindMember(Key)->value.GetBool() };
}

Package_t_Object Package::Object(const char* Key)
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The array of this package is empty"));
		return { false, Key, rapidjson::Value(rapidjson::kObjectType).GetObjectA() };
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The object of this package is empty"));
		return { false, Key, rapidjson::Value(rapidjson::kObjectType).GetObjectA() };
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Key ('%s') is not a part of this package"), Key);
		return { false, Key, rapidjson::Value(rapidjson::kObjectType).GetObjectA() };
	}

	if (pkg.FindMember(Key)->value.IsNull())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is null"), Key);
		return { false, Key, rapidjson::Value(rapidjson::kObjectType).GetObjectA() };
	}

	if (!pkg.FindMember(Key)->value.IsObject())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not an object"), Key);
		return { false, Key, rapidjson::Value(rapidjson::kObjectType).GetObjectA() };
	}

	return { true, Key, pkg.FindMember(Key)->value.GetObjectA() };
}

Package_t_Array Package::Array(const char* Key)
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The array of this package is empty"));
		return { false, Key, rapidjson::Value(rapidjson::kArrayType).GetArray() };
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The object of this package is empty"));
		return { false, Key, rapidjson::Value(rapidjson::kArrayType).GetArray() };
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Key ('%s') is not a part of this package"), Key);
		return { false, Key, rapidjson::Value(rapidjson::kArrayType).GetArray() };
	}

	if (pkg.FindMember(Key)->value.IsNull())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is null"), Key);
		return { false, Key, rapidjson::Value(rapidjson::kArrayType).GetArray() };
	}

	if (!pkg.FindMember(Key)->value.IsArray())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not an array"), Key);
		return { false, Key, rapidjson::Value(rapidjson::kArrayType).GetArray() };
	}

	return { true, Key, pkg.FindMember(Key)->value.GetArray() };
}

Package_RawData_t Package::RawData(const char* Key)
{
	for(auto& entry : rawData)
	{
		if (!strcmp(entry.key(), Key))
			return entry;
	}

	return Package_RawData_t();
}

NET_NAMESPACE_END
