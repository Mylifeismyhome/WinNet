#include "BackendAPI.h"

#define DISBALE_LOG
constexpr auto	URL = "";
#ifdef DEVELOPER_MODE
constexpr auto Path = "/API_DEV/Page/";
#else
constexpr auto Path = "/API/Page/";
#endif
constexpr auto ssl = true;

// key to allow communication between Server API & Server
constexpr auto API_KEY = "ncVFvyxTBHVr7Vgu9v9CDR3DnW8DYDaHDYcgZqFMehNpFKmW2TbQwqh8uEQEbLQvybhaXVerXHUPDsEqDj3hjg55jJX56vkC5zpyaF45nFBw4rKeJ2QpW5hvPjFLTTJ";

API::API(const char* url)
{
	lastResultMessage = nullptr;
	lastResultCode = 0;
	bodyContent = nullptr;
	bodyLen = 0;
	http = nullptr;
	https = nullptr;
	Init(url);
}

void API::Init(const char* filename)
{
	char httpFULLPATH[512];
	
	// set request URL
	if (ssl)
	{
		sprintf_s(httpFULLPATH, "https://%s%s%s", URL, Path, filename);
		https = new NET_HTTPS(httpFULLPATH, NET_SSL_METHOD::NET_SSL_METHOD_TLSv1_2_CLIENT);
	}
	else
	{
		sprintf_s(httpFULLPATH, "http://%s%s%s", URL, Path, filename);
		http = new NET_HTTP(httpFULLPATH);
	}

#ifndef DISBALE_LOG
	LOG_DEBUG("[API] - using %s", SSL ? "https" : "http");
	LOG_DEBUG("[API] - requesting host: %s, full Path: %s", URL, httpFULLPATH);
#endif
}

API::~API()
{
	FREE(lastResultMessage);
	FREE(bodyContent);
	delete http;
	delete https;
}

void API::AddParam(const char* name, const char* val) const
{
	if (http)
		http->AddParam(name, val);

	if (https)
		https->AddParam(name, val);

#ifndef DISBALE_LOG
	LOG_DEBUG("[API] - AddParam: %s, (string) %s", name, val);
#endif
}

void API::AddParam(const char* name, const int val) const
{
	if (http)
		http->AddParam(name, val);

	if (https)
		https->AddParam(name, val);

#ifndef DISBALE_LOG
	LOG_DEBUG("[API] - AddParam: %s, (int) %i", name, val);
#endif
}

void API::AddParam(const char* name, const long val) const
{
	if (http)
		http->AddParam(name, val);

	if (https)
		https->AddParam(name, val);

#ifndef DISBALE_LOG
	LOG_DEBUG("[API] - AddParam: %s, (long) %i", name, val);
#endif
}

void API::AddParam(const char* name, const long long val) const
{
	if (http)
		http->AddParam(name, val);

	if (https)
		https->AddParam(name, val);

#ifndef DISBALE_LOG
	LOG_DEBUG("[API] - AddParam: %s, (long long) %i", name, val);
#endif
}

void API::AddParam(const char* name, const float val) const
{
	if (http)
		http->AddParam(name, val);

	if (https)
		https->AddParam(name, val);

#ifndef DISBALE_LOG
	LOG_DEBUG("[API] - AddParam: %s, (float) %f", name, val);
#endif
}

int API::ParseResult(const char* str)
{
	result.Parse(str);

	const auto strLen = strlen(str);
	FREE(bodyContent);
	bodyContent = ALLOC<char>(strLen + 1);
	memcpy(bodyContent, str, strLen);
	bodyLen = static_cast<int>(strlen(str));

	// check Response Code
	if (!result.GetPackage().HasMember("Response"))
	{
		LOG_ERROR("[API] - Invalid Response: %s", bodyContent);
		return 9;
	}

	const auto objects = result.GetPackage().FindMember("Response")->value.GetArray();
	const auto resultCode = objects[0].GetInt();

	FREE(lastResultMessage);
	lastResultMessage = ALLOC<char>(objects[1].GetStringLength() + 1);
	memcpy(lastResultMessage, objects[1].GetString(), objects[1].GetStringLength());
	lastResultMessage[objects[1].GetStringLength()] = '\0';

	return resultCode;
}

bool API::Request()
{
	// Always add ServerKey
	if (http)
		http->AddParam("apikey", API_KEY);

	if (https)
		https->AddParam("apikey", API_KEY);

	if (http)
	{
		if (http->Post())
		{
			lastResultCode = ParseResult(http->GetBodyContent().data());
			return lastResultCode == 0;
		}

		LOG_ERROR("[API][HTTP] - Post() returned false, resultCode: %i!", http->GetResultCode());
		return false;
	}

	if (https)
	{
		if (https->Post())
		{
			lastResultCode = ParseResult(https->GetBodyContent().data());
			return lastResultCode == 0;
		}

		LOG_ERROR("[API][HTTPS] - Post() returned false, resultCode: %i!", https->GetResultCode());
		return false;
	}

	LOG_ERROR("[API] - http & https are invalid!?");
	return false;
}

Net::Package& API::Result()
{
	return result;
}

int API::GetResultCode() const
{
	return lastResultCode;
}

char* API::GetResultMessage() const
{
	if (!lastResultMessage)
		return (char*)"";

	return lastResultMessage;
}