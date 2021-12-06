#include <Net/Net/Package.h>

#ifdef DLL
NET_EXPORT_FUNCTION Net::Package::Interface* CreatePackage()
{
	return new Net::Package::Package();
}
#endif

Net::Package::Package_RawData_t::Package_RawData_t()
{
	memset(this->_key, NULL, 256);
	this->_data = nullptr;
	this->_size = NULL;
	this->_free_after_sent = false;
	this->_valid = false;
}

Net::Package::Package_RawData_t::Package_RawData_t(const char* name, byte* pointer, const size_t size)
{
	strcpy(this->_key, name);
	this->_data = pointer;
	this->_size = size;
	this->_free_after_sent = true;
	this->_valid = true;
}

Net::Package::Package_RawData_t::Package_RawData_t(const char* name, byte* pointer, const size_t size, const bool free_after_sent)
{
	strcpy(this->_key, name);
	this->_data = pointer;
	this->_size = size;
	this->_free_after_sent = free_after_sent;
	this->_valid = true;
}

bool Net::Package::Package_RawData_t::valid() const
{
	return _valid;
}

byte* Net::Package::Package_RawData_t::value() const
{
	return _data;
}

byte*& Net::Package::Package_RawData_t::value()
{
	return _data;
}

size_t Net::Package::Package_RawData_t::size() const
{
	return _size;
}

size_t& Net::Package::Package_RawData_t::size()
{
	return _size;
}

void Net::Package::Package_RawData_t::set_free(bool free)
{
	_free_after_sent = free;

}

bool Net::Package::Package_RawData_t::do_free() const
{
	return _free_after_sent;
}

const char* Net::Package::Package_RawData_t::key() const
{
	if (_key == nullptr) return CSTRING("");
	return _key;
}

void Net::Package::Package_RawData_t::set(byte* pointer)
{
	if (do_free())  FREE(_data);
	_data = pointer;
}

void Net::Package::Package_RawData_t::free()
{
	if (!this->_valid) return;

	// only free the passed reference if we allow it
	if (do_free()) FREE(this->_data);

	this->_data = nullptr;
	this->_size = NULL;

	this->_valid = false;
}

Net::Package::Package::Package()
{
	pkg.SetObject();
}

Net::Package::Package::~Package()
{
	/* free all raw data */
	for (auto& entry : rawData) entry.free();
}

void Net::Package::Package::Append(const char* Key, rapidjson::Value value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		pkg.FindMember(Key)->value = value;
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	if (!value.IsNull()) pkg.AddMember(key, value, pkg.GetAllocator());
}

void Net::Package::Package::Append(const char* Key, rapidjson::Value& value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		pkg.FindMember(Key)->value = value;
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	if (!value.IsNull()) pkg.AddMember(key, value, pkg.GetAllocator());
}

void Net::Package::Package::Append(const char* Key, const char* value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		rapidjson::Value v;
		v.Set<const char*>(value);
		pkg.FindMember(Key)->value = v;
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	rapidjson::Value v;
	v.Set<const char*>(value);

	if (!v.IsNull())
		pkg.AddMember(key, v, pkg.GetAllocator());
}

void Net::Package::Package::Append(const char* Key, char* value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		rapidjson::Value v;
		v.Set<const char*>(value);
		pkg.FindMember(Key)->value = v;
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	rapidjson::Value v;
	v.Set<const char*>(value);

	if (!v.IsNull())
		pkg.AddMember(key, v, pkg.GetAllocator());
}

void Net::Package::Package::Append(const char* Key, unsigned int value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		rapidjson::Value v;
		v.Set<unsigned int>(value);
		pkg.FindMember(Key)->value = v;
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	rapidjson::Value v;
	v.Set<unsigned int>(value);

	if (!v.IsNull())
		pkg.AddMember(key, v, pkg.GetAllocator());
}

void Net::Package::Package::Append(const char* Key, int value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		rapidjson::Value v;
		v.Set<int>(value);
		pkg.FindMember(Key)->value = v;
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	rapidjson::Value v;
	v.Set<int>(value);

	if (!v.IsNull())
		pkg.AddMember(key, v, pkg.GetAllocator());
}

void Net::Package::Package::Append(const char* Key, bool value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		rapidjson::Value v;
		v.Set<bool>(value);
		pkg.FindMember(Key)->value = v;
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	rapidjson::Value v;
	v.Set<bool>(value);

	if (!v.IsNull())
		pkg.AddMember(key, v, pkg.GetAllocator());
}

void Net::Package::Package::Append(const char* Key, long value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		rapidjson::Value v;
		v.Set<long>(value);
		pkg.FindMember(Key)->value = v;
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	rapidjson::Value v;
	v.Set<long>(value);

	if (!v.IsNull())
		pkg.AddMember(key, v, pkg.GetAllocator());
}

void Net::Package::Package::Append(const char* Key, float value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		rapidjson::Value v;
		v.Set<float>(value);
		pkg.FindMember(Key)->value = v;
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	rapidjson::Value v;
	v.Set<float>(value);

	if (!v.IsNull())
		pkg.AddMember(key, v, pkg.GetAllocator());
}

void Net::Package::Package::Append(const char* Key, double value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		rapidjson::Value v;
		v.Set<double>(value);
		pkg.FindMember(Key)->value = v;
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	rapidjson::Value v;
	v.Set<double>(value);

	if (!v.IsNull())
		pkg.AddMember(key, v, pkg.GetAllocator());
}

void Net::Package::Package::Append(const char* Key, size_t value)
{
	if (pkg.IsNull())
		pkg.SetObject();

	if (pkg.HasMember(Key))
	{
		rapidjson::Value v;
		v.Set<size_t>(value);
		pkg.FindMember(Key)->value = v;
		return;
	}

	rapidjson::Value key(Key, pkg.GetAllocator());
	rapidjson::Value v;
	v.Set<size_t>(value);

	if (!v.IsNull())
		pkg.AddMember(key, v, pkg.GetAllocator());
}

void Net::Package::Package::Append(const char* Key, BYTE* data, const size_t size, const bool free_after_sent)
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

	rawData.emplace_back(Net::Package::Package_RawData_t(Key, data, size, free_after_sent));
}

void Net::Package::Package::Append(Net::Package::Package_RawData_t& data)
{
	for (auto& entry : rawData)
	{
		if (!strcmp(entry.key(), data.key()))
		{
			if (data.do_free())
			{
				LOG_ERROR(CSTRING("Duplicated Key, buffer gets automaticly deleted from heap to avoid memory leaks"));
				data.free();
				return;
			}

			LOG_ERROR(CSTRING("Duplicated Key, buffer has not been deleted from heap"));
			return;
		}
	}

	rawData.emplace_back(data);
}

bool Net::Package::Package::Parse(const char* data)
{
	return !this->pkg.Parse(data).HasParseError();
}

void Net::Package::Package::SetPackage(const rapidjson::Document& doc)
{
	this->pkg.CopyFrom(doc, this->pkg.GetAllocator());
}

void Net::Package::Package::SetPackage(const rapidjson::Value& doc)
{
	this->pkg.CopyFrom(doc, this->pkg.GetAllocator());
}

void Net::Package::Package::SetPackage(const rapidjson::Document::Object& obj)
{
	this->pkg.Set(obj);
}

void Net::Package::Package::SetPackage(const rapidjson::Document::ValueIterator& arr)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	arr->Accept(writer);
	this->pkg.Parse(buffer.GetString());
}

void Net::Package::Package::SetRawData(const std::vector<Net::Package::Package_RawData_t>& data)
{
	rawData = data;
}

std::vector<Net::Package::Package_RawData_t>& Net::Package::Package::GetRawData()
{
	return rawData;
}

bool Net::Package::Package::HasRawData() const
{
	return !rawData.empty();
}

size_t Net::Package::Package::GetRawDataFullSize() const
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

rapidjson::Document& Net::Package::Package::GetPackage()
{
	return pkg;
}

std::string Net::Package::Package::StringifyPackage() const
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	pkg.Accept(writer);
	std::string outBuffer = buffer.GetString();
	return outBuffer;
}

Net::Package::Package_t<const char*> Net::Package::Package::String(const char* Key) const
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

	if (!pkg.FindMember(Key)->value.IsString())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not a string"), Key);
		return { false, Key, "" };
	}

	return { true, Key, pkg.FindMember(Key)->value.GetString() };
}

Net::Package::Package_t<int> Net::Package::Package::Int(const char* Key) const
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

Net::Package::Package_t<double> Net::Package::Package::Double(const char* Key) const
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

	if (!pkg.FindMember(Key)->value.IsDouble()
		&& !pkg.FindMember(Key)->value.IsFloat()
		&& !pkg.FindMember(Key)->value.IsInt())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not a double, float or integer"), Key);
		return { false, Key, 0 };
	}

	return { true, Key, (pkg.FindMember(Key)->value.IsDouble() ? pkg.FindMember(Key)->value.GetDouble() : (pkg.FindMember(Key)->value.IsFloat() ? static_cast<double>(pkg.FindMember(Key)->value.GetFloat()) : static_cast<double>(pkg.FindMember(Key)->value.GetInt()))) };
}

Net::Package::Package_t<float> Net::Package::Package::Float(const char* Key) const
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

	if (!pkg.FindMember(Key)->value.IsFloat()
		&& !pkg.FindMember(Key)->value.IsInt())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not a float or integer"), Key);
		return { false, Key, 0 };
	}

	return { true, Key,  (pkg.FindMember(Key)->value.IsFloat() ? pkg.FindMember(Key)->value.GetFloat() : static_cast<float>(pkg.FindMember(Key)->value.GetInt())) };
}

Net::Package::Package_t<int64> Net::Package::Package::Int64(const char* Key) const
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

Net::Package::Package_t<uint> Net::Package::Package::UINT(const char* Key) const
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

Net::Package::Package_t<uint64> Net::Package::Package::UINT64(const char* Key) const
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

Net::Package::Package_t<bool> Net::Package::Package::Boolean(const char* Key) const
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

Net::Package::Package_t_Object Net::Package::Package::Object(const char* Key)
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

Net::Package::Package_t_Array Net::Package::Package::Array(const char* Key)
{
	if (pkg.IsArray() && pkg.Empty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The array of this package is empty"));
		return Net::Package::Package_t_Array(false, Key, rapidjson::Value(rapidjson::kArrayType).GetArray());
	}

	if (pkg.IsObject() && pkg.ObjectEmpty())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - The object of this package is empty"));
		return Net::Package::Package_t_Array(false, Key, rapidjson::Value(rapidjson::kArrayType).GetArray());
	}

	if (!pkg.HasMember(Key))
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Key ('%s') is not a part of this package"), Key);
		return Net::Package::Package_t_Array(false, Key, rapidjson::Value(rapidjson::kArrayType).GetArray());
	}

	if (pkg.FindMember(Key)->value.IsNull())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is null"), Key);
		return Net::Package::Package_t_Array(false, Key, rapidjson::Value(rapidjson::kArrayType).GetArray());
	}

	if (!pkg.FindMember(Key)->value.IsArray())
	{
		LOG_DEBUG(CSTRING("[JSON][PACKAGE] - Value with the key pair of ('%s') is not an array"), Key);
		return Net::Package::Package_t_Array(false, Key, rapidjson::Value(rapidjson::kArrayType).GetArray());
	}

	return Net::Package::Package_t_Array(true, Key, pkg.FindMember(Key)->value.GetArray());
}

Net::Package::Binary_t Net::Package::Package::Binary(const char* Key)
{
	for (auto& entry : rawData)
	{
		if (!strcmp(entry.key(), Key))
			return Net::Package::Binary_t(&entry, true);
	}

	return Net::Package::Binary_t(nullptr, false);
}
