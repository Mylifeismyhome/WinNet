#include <Net/Net/NetPacket.h>

Net::RawData_t::RawData_t()
{
	memset(this->_key, 0, 256);
	this->_data = nullptr;
	this->_size = 0;
	this->_free_after_sent = false;
	this->_valid = false;
}

Net::RawData_t::RawData_t(const char* name, byte* pointer, const size_t size)
{
	strcpy(this->_key, name);
	this->_data = pointer;
	this->_size = size;
	this->_free_after_sent = true;
	this->_valid = true;
}

Net::RawData_t::RawData_t(const char* name, byte* pointer, const size_t size, const bool free_after_sent)
{
	strcpy(this->_key, name);
	this->_data = pointer;
	this->_size = size;
	this->_free_after_sent = free_after_sent;
	this->_valid = true;
}

bool Net::RawData_t::valid() const
{
	return _valid;
}

byte* Net::RawData_t::value() const
{
	return _data;
}

byte*& Net::RawData_t::value()
{
	return _data;
}

size_t Net::RawData_t::size() const
{
	return _size;
}

size_t& Net::RawData_t::size()
{
	return _size;
}

void Net::RawData_t::set_free(bool free)
{
	_free_after_sent = free;

}

bool Net::RawData_t::do_free() const
{
	return _free_after_sent;
}

const char* Net::RawData_t::key() const
{
	if (_key == nullptr) return CSTRING("");
	return _key;
}

void Net::RawData_t::set(byte* pointer)
{
	if (do_free()) FREE<byte>(_data);
	_data = pointer;
}

void Net::RawData_t::free()
{
	if (!this->_valid) return;

	// only free the passed reference if we allow it
	if (do_free()) FREE<byte>(this->_data);

	this->_data = nullptr;
	this->_size = 0;

	this->_valid = false;
}

Net::Packet::Packet::~Packet()
{
	/* free all raw data */
	for (auto& entry : this->raw) entry.free();
}

Net::Json::Document& Net::Packet::Data()
{
	return this->json;
}

void Net::Packet::AddRaw(const char* Key, BYTE* data, const size_t size, const bool free_after_sent)
{
	for (auto& entry : this->raw)
	{
		if (!strcmp(entry.key(), Key))
		{
			if (free_after_sent)
			{
				NET_LOG_ERROR(CSTRING("Duplicated Key, buffer gets automaticly deleted from heap to avoid memory leaks"));
				FREE<byte>(data);
				return;
			}

			NET_LOG_ERROR(CSTRING("Duplicated Key, buffer has not been deleted from heap"));
			return;
		}
	}

	this->raw.emplace_back(Net::RawData_t(Key, data, size, free_after_sent));
}

void Net::Packet::AddRaw(Net::RawData_t& raw)
{
	for (auto& entry : this->raw)
	{
		if (!strcmp(entry.key(), raw.key()))
		{
			if (raw.do_free())
			{
				NET_LOG_ERROR(CSTRING("Duplicated Key, buffer gets automaticly deleted from heap to avoid memory leaks"));
				raw.free();
				return;
			}

			NET_LOG_ERROR(CSTRING("Duplicated Key, buffer has not been deleted from heap"));
			return;
		}
	}

	this->raw.emplace_back(raw);
}

bool Net::Packet::Deserialize(char* data)
{
	return this->json.Deserialize(data);
}

bool Net::Packet::Deserialize(const char* data)
{
	return this->json.Deserialize(data);
}

void Net::Packet::SetJson(Net::Json::Document& doc)
{
	this->json = doc;
}

void Net::Packet::SetRaw(const std::vector<Net::RawData_t>& raw)
{
	this->raw = raw;
}

std::vector<Net::RawData_t>& Net::Packet::Packet::GetRawData()
{
	return this->raw;
}

bool Net::Packet::Packet::HasRawData() const
{
	return !this->raw.empty();
}

size_t Net::Packet::Packet::GetRawDataFullSize() const
{
	size_t size = 0;
	for (auto& entry : this->raw)
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

Net::String Net::Packet::Stringify()
{
	return this->json.Serialize(Net::Json::SerializeType::UNFORMATTED);
}
