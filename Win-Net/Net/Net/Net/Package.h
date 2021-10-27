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

		class NET_EXPORT_CLASS Interface
		{
		public:
			virtual void AppendRawData(const char* Key, BYTE* data, const size_t size, const bool free_after_sent = true) = 0;
			virtual void AppendRawData(Net::Package::Package_RawData_t& data) = 0;

			virtual void RewriteRawData(const char* Key, BYTE* data) = 0;

			virtual bool Parse(const char* data) = 0;

			virtual void SetPackage(const rapidjson::Document& doc) = 0;
			virtual void SetPackage(const rapidjson::Value& doc) = 0;
			virtual void SetPackage(const rapidjson::Document::Object& obj) = 0;
			virtual void SetPackage(const rapidjson::Document::ValueIterator& arr) = 0;
			virtual void SetRawData(const std::vector<Net::Package::Package_RawData_t>& data) = 0;

			virtual std::vector<Net::Package::Package_RawData_t>& GetRawData() = 0;
			virtual bool HasRawData() const = 0;
			virtual size_t GetRawDataFullSize() const = 0;

			virtual rapidjson::Document& GetPackage() = 0;
			virtual std::string StringifyPackage() const = 0;
			virtual Net::Package::Package_t<const char*> String(const char*) const = 0;
			virtual Net::Package::Package_t<int> Int(const char*) const = 0;
			virtual Net::Package::Package_t<double> Double(const char*) const = 0;
			virtual Net::Package::Package_t<float> Float(const char*) const = 0;
			virtual Net::Package::Package_t<int64> Int64(const char*) const = 0;
			virtual Net::Package::Package_t<uint> UINT(const char*) const = 0;
			virtual Net::Package::Package_t<uint64> UINT64(const char*) const = 0;
			virtual Net::Package::Package_t<bool> Boolean(const char*) const = 0;
			virtual Net::Package::Package_t_Object Object(const char*) = 0;
			virtual Net::Package::Package_t_Array Array(const char*) = 0;
			virtual Net::Package::Package_RawData_t& RawData(const char*) = 0;
		};

		class NET_EXPORT_CLASS Package : public Interface
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

			void AppendRawData(const char* Key, BYTE* data, const size_t size, const bool free_after_sent = true) override;
			void AppendRawData(Net::Package::Package_RawData_t& data) override;

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

			void RewriteRawData(const char* Key, BYTE* data) override;

			bool Parse(const char* data) override;

			void SetPackage(const rapidjson::Document& doc) override;
			void SetPackage(const rapidjson::Value& doc) override;
			void SetPackage(const rapidjson::Document::Object& obj) override;
			void SetPackage(const rapidjson::Document::ValueIterator& arr) override;
			void SetRawData(const std::vector<Net::Package::Package_RawData_t>& data) override;

			std::vector<Net::Package::Package_RawData_t>& GetRawData() override;
			bool HasRawData() const override;
			size_t GetRawDataFullSize() const override;

			rapidjson::Document& GetPackage() override;
			std::string StringifyPackage() const override;
			Net::Package::Package_t<const char*> String(const char*) const override;
			Net::Package::Package_t<int> Int(const char*) const override;
			Net::Package::Package_t<double> Double(const char*) const override;
			Net::Package::Package_t<float> Float(const char*) const override;
			Net::Package::Package_t<int64> Int64(const char*) const override;
			Net::Package::Package_t<uint> UINT(const char*) const override;
			Net::Package::Package_t<uint64> UINT64(const char*) const override;
			Net::Package::Package_t<bool> Boolean(const char*) const override;
			Net::Package::Package_t_Object Object(const char*) override;
			Net::Package::Package_t_Array Array(const char*) override;
			Net::Package::Package_RawData_t& RawData(const char*) override;
		};
	}
}

NET_DSA_END
