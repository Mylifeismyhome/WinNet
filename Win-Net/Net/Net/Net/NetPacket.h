#pragma once
#define NET_PACKET Net::Packet
#define NET_PACKET_JOIN(pkg1, pkg2) \
pkg1.SetJson(pkg2.Data()); \
pkg1.SetRaw(pkg2.GetRawData());

///////////////////////////////////////////////////////////
///														///
///		  FAST PACKAGE ID IMPLEMENTATION				///
///														///
//////////////////////////////////////////////////////////
#define NET_DEFINE_PACKAGES(name, first, ...) \
namespace name \
{ \
	enum T##name \
	{ \
	first = NET_LAST_PACKAGE_ID, \
	__VA_ARGS__ \
	}; \
}

#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>
#include <Net/Net/NetNativePacket.h>
#include <Net/Net/NetJson.h>

#include <Net/assets/assets.h>

NET_DSA_BEGIN

namespace Net
{
	class RawData_t
	{
		char _key[256];
		byte* _data;
		size_t _size;
		bool _free_after_sent; /* by default this value is set to TRUE */
		bool _valid;

	public:
		RawData_t();
		RawData_t(const char* name, byte* pointer, const size_t size);
		RawData_t(const char* name, byte* pointer, const size_t size, const bool free_after_sent);

		bool valid() const;
		byte* value() const;
		byte*& value();
		size_t size() const;
		size_t& size();
		void set_free(bool free);
		bool do_free() const;
		const char* key() const;
		void set(byte* pointer);
		void free();
	};

	class Packet
	{
		Net::Json::Document json;
		std::vector<Net::RawData_t> raw;

	public:
		~Packet();

		Net::Json::BasicValueRead operator[](const char* key)
		{
			return json[key];
		}

		Net::Json::Document& Data();

		void AddRaw(const char* Key, BYTE* data, const size_t size, const bool free_after_sent = true);
		void AddRaw(Net::RawData_t& raw);

		bool Deserialize(const char* data);

		void SetJson(Net::Json::Document& doc);
		void SetRaw(const std::vector<Net::RawData_t>& raw);

		std::vector<Net::RawData_t>& GetRawData();
		bool HasRawData() const;
		size_t GetRawDataFullSize() const;

		Net::String Stringify();
	};
}

NET_DSA_END
