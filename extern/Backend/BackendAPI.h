#pragma once
#include <config.h>
#include <assets/assets.h>
#include <assets/web/http.h>
#include <pugiXML/src/pugixml.hpp>
#include <Net/Package.h>

class API
{
	NET_HTTP* http;
	NET_HTTPS* https;

	int	lastResultCode;
	char* lastResultMessage;

	char* bodyContent;
	int bodyLen;

	Net::Package result;

	int ParseResult(const char*);

	void Init(const char*);

public:
	API(const char*);
	~API();

	void AddParam(const char*, const char*) const;
	void AddParam(const char*, int) const;
	void AddParam(const char*, long) const;
	void AddParam(const char*, long long) const;
	void AddParam(const char*, float) const;

	bool Request();

	Net::Package& Result();
	int GetResultCode() const;
	char* GetResultMessage() const;
};