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

namespace Net
{
	namespace Package
	{
		class Package_RawData_t
		{
			char _key[256];
			byte* _data;
			size_t _size;
			bool _free_after_sent; /* by default this value is set to TRUE */
			bool _valid;

		public:
			Package_RawData_t();
			Package_RawData_t(const char* name, byte* pointer, const size_t size);
			Package_RawData_t(const char* name, byte* pointer, const size_t size, const bool free_after_sent);

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

		template<typename TYPE>
		class Package_t
		{
			bool _valid;
			char _name[256];
			TYPE _value;

		public:
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
		};

		class Package_t_Object
		{
			bool _valid;
			char _name[256];
			rapidjson::Value _value;

		public:
			Package_t_Object(const bool _valid, const char* _name, rapidjson::Value _value);
			bool valid() const;
			const char* name() const;
			rapidjson::Value::Object value();
		};
		
		class Package_t_Array
		{
			bool _valid;
			char _name[256];
			rapidjson::Value _value;

		public:
			Package_t_Array(const bool _valid, const char* _name, rapidjson::Value _value);

			bool valid() const;
			const char* name() const;
			rapidjson::Value::Array value();
		};

		class Package
		{
			rapidjson::Document pkg;
			std::vector<Net::Package::Package_RawData_t> rawData;

		public:
			Package();
			~Package();

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

			void AppendRawData(const char* Key, BYTE* data, const size_t size, const bool free_after_sent = true);
			void AppendRawData(Net::Package::Package_RawData_t& data);

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

			void RewriteRawData(const char* Key, BYTE* data);

			bool Parse(const char* data);

			void SetPackage(const rapidjson::Document& doc);
			void SetPackage(const rapidjson::Value& doc);
			void SetPackage(const rapidjson::Document::Object& obj);
			void SetPackage(const rapidjson::Document::ValueIterator& arr);
			void SetRawData(const std::vector<Net::Package::Package_RawData_t>& data);

			std::vector<Net::Package::Package_RawData_t>& GetRawData();
			bool HasRawData() const;
			size_t GetRawDataFullSize() const;

			rapidjson::Document& GetPackage();
			std::string StringifyPackage() const;
			Net::Package::Package_t<const char*> String(const char*) const;
			Net::Package::Package_t<int> Int(const char*) const;
			Net::Package::Package_t<double> Double(const char*) const;
			Net::Package::Package_t<float> Float(const char*) const;
			Net::Package::Package_t<int64> Int64(const char*) const;
			Net::Package::Package_t<uint> UINT(const char*) const;
			Net::Package::Package_t<uint64> UINT64(const char*) const;
			Net::Package::Package_t<bool> Boolean(const char*) const;
			Net::Package::Package_t_Object Object(const char*);
			Net::Package::Package_t_Array Array(const char*);
			Net::Package::Package_RawData_t& RawData(const char*);
		};
	}
}

NET_DSA_END
