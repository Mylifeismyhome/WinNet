#pragma once
#include <Net/Net.h>
#include <assets/manager/logmanager.h>

#define MYSQL_CONFIG const MYSQL_CON
#define MYSQL MYSQL

/*
 * MYSQL_FREE => Free single result pointer
 * MYSQL_FLUSH => Free multiplie result pointers
 */
#define MYSQL_FREE(x) delete x
#define MYSQL_FLUSH(x) \
			if(x.IsValid()) \
			{ \
				MYSQL_FREE(x.result); \
			}
#define MYSQL_MULTI_FLUSH(x) \
			if(x.IsValid()) \
			{ \
				for (auto& arr : x.results) \
				{ \
					if (arr.IsValid()) \
					{ \
						MYSQL_FREE(arr.result); \
					} \
				} \
			}

 /*
  * MYSQL_ITER => Iterates through multi results
  * MYSQL_NEXT => Iterates through result rows
  */
#define MYSQL_ITER(x, y) for (auto x : y.Get())
#define MYSQL_NEXT(x) while (x.result->next())

#define QUERY MYSQL_QUERY
#define MULTIQUERY MYSQL_MUTLIQUERY

#define FREESTRING(sql) delete[] sql; sql = nullptr;

#include <MYSQL/include/jdbc/cppconn/driver.h>
#include <MYSQL/include/jdbc/cppconn/exception.h>
#include <MYSQL/include/jdbc/cppconn/resultset.h>
#include <MYSQL/include/jdbc/cppconn/statement.h>

#include <iterator>
#include <vector>
#include <cstdarg>

  /*
  * Connetion config
  */
BEGIN_CLASS(MYSQL_CON)
char conIP[40];
short conPort;

char conUsername[32];
char conPassword[256];
char conDB[32];

CLASS_PUBLIC
CLASS_CONSTRUCTUR(MYSQL_CON);
CLASS_CONSTRUCTUR(MYSQL_CON, const char* ip, const short port, const char* username, const char* password = "", const char* db = "test");

char* getIP();
short getPort() const;
char* getUsername();
char* getPassword();
char* getDB();
END_CLASS;

/*
* RESULTS
*/
BEGIN_CLASS(MYSQL_RESULT)
bool valid;
char name[32];
sql::ResultSet* result;

CLASS_PUBLIC
CLASS_CONSTRUCTUR(MYSQL_RESULT);
CLASS_CONSTRUCTUR(MYSQL_RESULT, const char* n, sql::ResultSet* res);

bool IsValid() const;
bool Empty() const;
size_t Count() const;
sql::ResultSet* Get() const;
END_CLASS;

/*
* QUERY
*/
BEGIN_CLASS(MYSQL_QUERY)
char name[32];
char* query;

CLASS_PUBLIC
CLASS_CONSTRUCTUR(MYSQL_QUERY, const char* n, char* q);
CLASS_CONSTRUCTUR(MYSQL_QUERY, char* q);

char* Get() const;
char* getName();
void Free();
END_CLASS;


/*
* MULTI QUERY
*/
BEGIN_CLASS(MYSQL_MUTLIQUERY)
std::vector<MYSQL_QUERY> query;

CLASS_PUBLIC
MYSQL_MUTLIQUERY() = default;
CLASS_CONSTRUCTUR(MYSQL_MUTLIQUERY, const std::vector<MYSQL_QUERY> q);

void Add(const char* name, char*& q);
void Add(const MYSQL_QUERY q);
void Add(MYSQL_QUERY & q);
bool IsValid() const;
std::vector<MYSQL_QUERY> Get() const;
END_CLASS;

/*
* MULTI RESULT
*/
BEGIN_CLASS(MYSQL_MULTIRESULT)
std::vector<MYSQL_RESULT> results;

CLASS_PUBLIC
void Add(MYSQL_RESULT res);
void Add(MYSQL_RESULT & res);
bool IsValid() const;
std::vector<MYSQL_RESULT> Get() const;
END_CLASS;

BEGIN_CLASS(MYSQL)
sql::Driver* msqldriver = nullptr;
sql::Connection* msqlcon = nullptr;
MYSQL_CON conConfig;

bool setup();

char* lastError = nullptr;

CLASS_PUBLIC
explicit MYSQL(const MYSQL_CON conConfig)
{
	this->conConfig = conConfig;
	if (!setup())
		return;
}

CLASS_DESTRUCTUR(MYSQL);

bool connect();
bool disconnect();

void SetLastError(const char*);
char* GetLastError() const;

MYSQL_RESULT query(char*);
MYSQL_RESULT query(MYSQL_QUERY);
MYSQL_MULTIRESULT multiQuery(MYSQL_MUTLIQUERY);
END_CLASS;

static char* SQLString(sql::SQLString string, ...)
{
	va_list vaArgs;
	va_start(vaArgs, string);
	const size_t size = std::vsnprintf(nullptr, 0, string.c_str(), vaArgs);
	std::vector<char> str(size + 1);
	std::vsnprintf(str.data(), str.size(), string.c_str(), vaArgs);
	va_end(vaArgs);

	auto* res = new char[str.size() + 1];
	memcpy(res, str.data(), str.size());
	res[str.size()] = '\0';
	return res;
}