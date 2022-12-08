/*
	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

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

		bool Deserialize(char* data);
		bool Deserialize(const char* data);

		void SetJson(Net::Json::Document& doc);
		void SetRaw(const std::vector<Net::RawData_t>& raw);

		std::vector<Net::RawData_t>& GetRawData();
		bool HasRawData() const;
		size_t GetRawDataFullSize() const;
		Net::RawData_t* GetRaw(const char* Key);

		Net::String Stringify();
	};
}

NET_DSA_END
