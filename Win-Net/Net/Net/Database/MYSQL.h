#pragma once
#include <Net/Net/Net.h>

#ifdef NET_USE_MYSQL_SUPPORT
#include <Net/Net/NetString.h>
#include <Net/assets/manager/logmanager.h>
#include <mutex>

NET_DSA_BEGIN

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
	class MYSQL_CON
{
	char conIP[40];
	short conPort;

	char conUsername[32];
	char conPassword[256];
	char conDB[32];

public:
	MYSQL_CON();
	MYSQL_CON(const char* ip, const short port, const char* username, const char* password = "", const char* db = CSTRING("database_name_here"));

	char* getIP();
	short getPort() const;
	char* getUsername();
	char* getPassword();
	char* getDB();
};

/*
* RESULTS
*/
class MYSQL_RESULT
{
	bool valid;
	char name[32];
	sql::ResultSet* result;

public:
	MYSQL_RESULT();
	MYSQL_RESULT(const char* n, sql::ResultSet* res);
	~MYSQL_RESULT();

	bool IsValid() const;
	bool Empty() const;
	size_t Count() const;
	sql::ResultSet* Get() const;
};

/*
* QUERY
*/
class MYSQL_QUERY
{
	char name[32];
	char* query;

public:
	MYSQL_QUERY(const char* n, char* q);
	MYSQL_QUERY(char* q);

	char* Get() const;
	char* getName();
	void Free();
};


/*
* MULTI QUERY
*/
class MYSQL_MUTLIQUERY
{
	std::vector<MYSQL_QUERY> query;

public:
	MYSQL_MUTLIQUERY() = default;
	MYSQL_MUTLIQUERY(std::vector<MYSQL_QUERY>& q);

	void Add(const char* name, char*& q);
	void Add(const MYSQL_QUERY q);
	void Add(MYSQL_QUERY& q);
	bool IsValid() const;
	std::vector<MYSQL_QUERY> Get() const;
};

/*
* MULTI RESULT
*/
class MYSQL_MULTIRESULT
{
	std::vector<MYSQL_RESULT> results;

public:
	void Add(MYSQL_RESULT res);
	void Add(MYSQL_RESULT& res);
	bool IsValid() const;
	std::vector<MYSQL_RESULT> Get() const;
};

class MYSQL
{
	sql::Driver* msqldriver = nullptr;
	sql::Connection* msqlcon = nullptr;
	MYSQL_CON conConfig;
	std::recursive_mutex critical;

	bool setup();

	char* lastError = nullptr;

public:
	explicit MYSQL(const MYSQL_CON conConfig)
	{
		this->conConfig = conConfig;
		if (!setup())
			return;
	}
	~MYSQL();

	bool connect();
	bool disconnect();
	bool reconnect();

	void lock();
	void unlock();

	void SetLastError(const char*);
	char* GetLastError() const;

	MYSQL_RESULT query(char*, bool = false);
	MYSQL_RESULT query(MYSQL_QUERY, bool = false);
	MYSQL_MULTIRESULT multiQuery(MYSQL_MUTLIQUERY, bool = false);
};

char* SQLString(const char* string, ...);

NET_DSA_END
#endif
