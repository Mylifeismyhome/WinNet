#pragma once
#define NET_PACKAGE Net::Package::Package&

#define NET_JOIN_PACKAGE(PKGOLD, PKGNEW) \
	Package PKGNEW; \
	PKGNEW.SetPackage(PKGOLD.GetPackage());

///////////////////////////////////////////////////////////
///							///
///		  FAST PACKAGE ID IMPLEMENTATION	///
///							///
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


		class Binary_t
		{
			Package_RawData_t* ptr_data;
			bool is_valid;

		public:
			Binary_t(Package_RawData_t* ptr_data, bool is_valid)
			{
				this->ptr_data = ptr_data;
				this->is_valid = is_valid;
			}

			Package_RawData_t& data() { return *ptr_data; }
			bool valid() const { return is_valid; }
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
				return _value.GetObjectA();
			}
		};

		class Package_t_Array
		{
			bool _valid;
			char _name[256];
			rapidjson::Value _value;

		public:
			Package_t_Array(const bool _valid, const char* _name, rapidjson::Value::Array _value)
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
		};

		class NET_EXPORT_CLASS Interface
		{
		public:
			Interface() = default;

			virtual void Append(const char* Key, rapidjson::Value value) = 0;
			virtual void Append(const char* Key, rapidjson::Value& value) = 0;
			virtual void Append(const char* Key, const char* value) = 0;
			virtual void Append(const char* Key, char* value) = 0;
			virtual void Append(const char* Key, unsigned int value) = 0;
			virtual void Append(const char* Key, int value) = 0;
			virtual void Append(const char* Key, long value) = 0;
			virtual void Append(const char* Key, float value) = 0;
			virtual void Append(const char* Key, double value) = 0;
			virtual void Append(const char* Key, BYTE* data, const size_t size, const bool free_after_sent = true) = 0;
			virtual void Append(Net::Package::Package_RawData_t& data) = 0;

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
			virtual Net::Package::Binary_t Binary(const char*) = 0;
		};

		class Package : public Interface
		{
			rapidjson::Document pkg;
			std::vector<Net::Package::Package_RawData_t> rawData;

		public:
			Package();
			~Package();

			void Append(const char* Key, rapidjson::Value value) override;
			void Append(const char* Key, rapidjson::Value& value) override;
			void Append(const char* Key, const char* value) override;
			void Append(const char* Key, char* value) override;
			void Append(const char* Key, unsigned int value) override;
			void Append(const char* Key, int value) override;
			void Append(const char* Key, long value) override;
			void Append(const char* Key, float value) override;
			void Append(const char* Key, double value) override;
			void Append(const char* Key, BYTE* data, const size_t size, const bool free_after_sent = true) override;
			void Append(Net::Package::Package_RawData_t& data) override;

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
			Net::Package::Binary_t Binary(const char*) override;
		};
	}
}

NET_DSA_END
