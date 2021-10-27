#pragma once
#define NET_HTTP Net::Web::HTTP
#define NET_HTTPS Net::Web::HTTPS
#define NET_USER_AGENT CSTRING("Net.Net/100")

#include <Net/Net/Net.h>
#include <Net/assets/assets.h>
#include <openssl/err.h>

#ifdef BUILD_LINUX
#define LAST_ERROR errno
#else
#define LAST_ERROR WSAGetLastError()
#endif

NET_DSA_BEGIN

namespace Net
{
	namespace Web
	{
		struct HeaderData_t
		{
			char* key;
			char* value;

			explicit HeaderData_t(const char*, char*, size_t = INVALID_SIZE);
			explicit HeaderData_t(const char*, const char*, size_t = INVALID_SIZE);
			explicit HeaderData_t(const char*, unsigned char*, size_t = INVALID_SIZE);

			void free();
		};

		class NET_EXPORT_CLASS Interface
		{
		public:
			virtual std::string& GetProtocol() = 0;
			virtual std::string& GetURL() = 0;
			virtual std::string& GetPath() = 0;
			virtual short GetPort() const = 0;

			virtual std::string& GetHeaderContent() = 0;
			virtual std::string& GetBodyContent() = 0;
			virtual std::string& GetRawData() = 0;
			virtual int GetResultCode() const = 0;

			virtual void URL_Encode(char*&) const = 0;
			virtual void URL_Encode(std::string&) const = 0;
			virtual void URL_Decode(char*&) const = 0;
			virtual void URL_Decode(std::string&) const = 0;

			virtual void AddParam(const char*, int) = 0;
			virtual void AddParam(const char*, long) = 0;
			virtual void AddParam(const char*, long long) = 0;
			virtual void AddParam(const char*, float) = 0;
			virtual void AddParam(const char*, const char*) = 0;
			virtual void SetJSON(const char*) = 0;

			virtual void AddHeader(const char*, char*, size_t = INVALID_SIZE) = 0;
			virtual void AddHeader(const char*, const char*, size_t = INVALID_SIZE) = 0;
			virtual void AddHeader(const char*, unsigned char*, size_t = INVALID_SIZE) = 0;

			virtual bool Get() = 0;
			virtual bool Post() = 0;
		};

		struct Network_t
		{
			byte dataReceive[NET_OPT_DEFAULT_MAX_PACKET_SIZE];
			CPOINTER<byte> data;
			size_t data_size;
			size_t data_full_size;

			Network_t();

			void AllocData(size_t);
			void clearData();
		};

		class NET_EXPORT_CLASS Head : public Interface
		{
		protected:
			std::string protocol;
			std::string url;
			std::string path;
			short port;
			Network_t network;
			std::vector<HeaderData_t> headerData;

			SOCKET connectSocket;
			struct addrinfo* connectSocketAddr;

			std::map<std::string, int>INT_Parameters;
			std::map<std::string, long>LONG_Parameters;
			std::map<std::string, long long>LONGLONG_Parameters;
			std::map<std::string, float>FLOAT_Parameters;
			std::map<std::string, const char*>STRING_Parameters;
			std::string STRING_JSON;
			std::string GetParameters() const;

			bool ParseResult();

			std::string rawData;
			std::string headContent;
			std::string bodyContent;
			int resultCode;

			void ShutdownSocket(int = SOCKET_RDWR) const;

			void SetRawData(std::string);
			void SetHeaderContent(std::string);
			void SetBodyContent(std::string);
			void SetResultCode(int);

			SOCKET GetSocket() const;

		public:
			Head();
			~Head();

			std::string& GetProtocol() override;
			std::string& GetURL() override;
			std::string& GetPath() override;
			short GetPort() const override;

			std::string& GetHeaderContent() override;
			std::string& GetBodyContent() override;
			std::string& GetRawData() override;
			int GetResultCode() const override;

			void URL_Encode(char*&) const override;
			void URL_Encode(std::string&) const override;
			void URL_Decode(char*&) const override;
			void URL_Decode(std::string&) const override;

			void AddParam(const char*, int) override;
			void AddParam(const char*, long) override;
			void AddParam(const char*, long long) override;
			void AddParam(const char*, float) override;
			void AddParam(const char*, const char*) override;
			void SetJSON(const char*) override;

			void AddHeader(const char*, char*, size_t = INVALID_SIZE) override;
			void AddHeader(const char*, const char*, size_t = INVALID_SIZE) override;
			void AddHeader(const char*, unsigned char*, size_t = INVALID_SIZE) override;
		};

		class NET_EXPORT_CLASS HTTP : public Head
		{
			friend class Head;

			bool Init(const char*);
			bool Inited;

			size_t DoSend(std::string&) const;
			size_t DoReceive();

		public:
			explicit HTTP(const char*);
			~HTTP();

			bool IsInited() const;

			// Methodes
			bool Get() override;
			bool Post() override;
		};

		class NET_EXPORT_CLASS HTTPS : public Head
		{
			friend class Head;
			SSL_CTX* ctx;
			SSL* ssl;

			bool Init(const char*, ssl::NET_SSL_METHOD);
			bool Inited;

			size_t DoSend(std::string&) const;
			size_t DoReceive();

		public:
			explicit HTTPS(const char*, ssl::NET_SSL_METHOD = ssl::NET_SSL_METHOD::NET_SSL_METHOD_TLS);
			~HTTPS();

			bool IsInited() const;

			// Methodes
			bool Get() override;
			bool Post() override;
		};
	}
}

NET_DSA_END