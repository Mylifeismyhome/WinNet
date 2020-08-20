#pragma once
#include "Net/ErrorCodes/ErrorCodes.hpp"

/* Macros to all Codes */
struct ErrorCode_T
{
	int Code;
	char Message[256];
};

enum ErrorCodes
{
	ERR_TEST = NetErrorCodes::LAST_NET_ERROR_CODE + 1,
};

static ErrorCode_T ErrorCode_L[] =
{
	{ErrorCodes::ERR_TEST, "MSG"},
};

static char* GetErrorMessage(const int code)
{
	for (auto& val : ErrorCode_L)
	{
		if (val.Code == code)
			return val.Message;
	}

	return (char*)"UNKNOWN";
}