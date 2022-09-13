#include "MYSQL.h"

#ifdef NET_USE_MYSQL_SUPPORT
char* SQLString(const char* string, ...)
{
	va_list vaArgs;

#ifdef BUILD_LINUX
	va_start(vaArgs, string);
	const size_t size = std::vsnprintf(nullptr, 0, string, vaArgs);
	va_end(vaArgs);

	va_start(vaArgs, string);
	std::vector<char> str(size + 1);
	std::vsnprintf(str.data(), str.size(), string, vaArgs);
	va_end(vaArgs);
#else
	va_start(vaArgs, string);
	const size_t size = std::vsnprintf(nullptr, 0, string, vaArgs);
	std::vector<char> str(size + 1);
	std::vsnprintf(str.data(), str.size(), string, vaArgs);
	va_end(vaArgs);
#endif

	auto* res = new char[str.size() + 1];
	memcpy(res, str.data(), str.size());
	res[str.size()] = '\0';
	return res;
}

MYSQL_CON::MYSQL_CON()
{
	sprintf(conIP, CSTRING(""));
	conPort = 0;

	sprintf(conUsername, CSTRING(""));
	sprintf(conPassword, CSTRING(""));
	sprintf(conDB, CSTRING(""));
}

MYSQL_CON::MYSQL_CON(const char* ip, const short port, const char* username, const char* password, const char* db)
{
	sprintf(conIP, ip);
	conPort = port;

	sprintf(conUsername, username);
	sprintf(conPassword, password);
	sprintf(conDB, db);
}

char* MYSQL_CON::getIP()
{
	return conIP;
}

short MYSQL_CON::getPort() const
{
	return conPort;
}

char* MYSQL_CON::getUsername()
{
	return conUsername;
}

char* MYSQL_CON::getPassword()
{
	return conPassword;
}

char* MYSQL_CON::getDB()
{
	return conDB;
}

MYSQL_RESULT::MYSQL_RESULT()
{
	sprintf(name, CSTRING(""));
	result = nullptr;
	valid = false;
}

MYSQL_RESULT::MYSQL_RESULT(const char* n, sql::ResultSet* res)
{
	sprintf(name, n);
	result = res;
	valid = true;
}

MYSQL_RESULT::~MYSQL_RESULT()
{
	delete result;
}

bool MYSQL_RESULT::IsValid() const
{
	return valid;
}

bool MYSQL_RESULT::Empty() const
{
	return result->rowsCount() == 0 ? true : false;
}

size_t MYSQL_RESULT::Count() const
{
	return result->rowsCount();
}

sql::ResultSet* MYSQL_RESULT::Get() const
{
	return result;
}

MYSQL_QUERY::MYSQL_QUERY(const char* n, char* q)
{
	sprintf(name, n);
	query = q;
}

MYSQL_QUERY::MYSQL_QUERY(char* q)
{
	sprintf(name, CSTRING(""));
	query = q;
}

char* MYSQL_QUERY::Get() const
{
	return query;
}

char* MYSQL_QUERY::getName()
{
	return name;
}

void MYSQL_QUERY::Free()
{
	FREESTRING(query);
}

MYSQL_MUTLIQUERY::MYSQL_MUTLIQUERY(std::vector<MYSQL_QUERY>& q)
{
	query = q;
}

void MYSQL_MUTLIQUERY::Add(const char* name, char*& q)
{
	query.emplace_back(MYSQL_QUERY{ name, q });
}

void MYSQL_MUTLIQUERY::Add(const MYSQL_QUERY q)
{
	query.emplace_back(q);
}

void MYSQL_MUTLIQUERY::Add(MYSQL_QUERY& q)
{
	query.emplace_back(q);
}

bool MYSQL_MUTLIQUERY::IsValid() const
{
	return query.empty();
}

std::vector<MYSQL_QUERY> MYSQL_MUTLIQUERY::Get() const
{
	return query;
}

void MYSQL_MULTIRESULT::Add(const MYSQL_RESULT res)
{
	results.emplace_back(res);
}

void MYSQL_MULTIRESULT::Add(MYSQL_RESULT& res)
{
	results.emplace_back(res);
}

bool MYSQL_MULTIRESULT::IsValid() const
{
	return results.empty();
}

std::vector<MYSQL_RESULT> MYSQL_MULTIRESULT::Get() const
{
	return results;
}

MYSQL::~MYSQL()
{
	if(msqlcon)
	{
		disconnect();
		delete msqlcon;
	}

	msqldriver = nullptr;
	FREE(lastError);
}

bool MYSQL::setup()
{
	msqldriver = get_driver_instance();
	if (!msqldriver)
	{
		NET_LOG_ERROR(CSTRING("[MYSQL] - Mysql driver instance does not exists"));
		return false;
	}

	return true;
}

bool MYSQL::connect()
{
	try
	{
		char constr[128];
		sprintf(constr, CSTRING("tcp://%s:%i"), conConfig.getIP(), conConfig.getPort());

		msqlcon = msqldriver->connect(constr, conConfig.getUsername(), conConfig.getPassword());
		if (!msqlcon)
			return false;

		msqlcon->setSchema(conConfig.getDB());
		return msqlcon->isValid();
	}
	catch (sql::SQLException& e)
	{
		SetLastError(e.what());
		NET_LOG_ERROR(CSTRING("[MYSQL] - %s => {%i}"), e.what(), e.getErrorCode());
	}

	return false;
}

bool MYSQL::disconnect()
{
	if (!msqlcon)
		return false;

	try
	{
		msqlcon->close();
		const auto closed = msqlcon->isClosed();
		return closed;
	}
	catch (sql::SQLException& e)
	{
		SetLastError(e.what());
		NET_LOG_ERROR(CSTRING("[MYSQL] - %s => {%i}"), e.what(), e.getErrorCode());
	}

	return false;
}

bool MYSQL::reconnect()
{
	disconnect();
	const auto res = connect();
	if (!res)
		NET_LOG_ERROR(CSTRING("[MYSQL] - Failure on re-estabilishing connection"));

	return res;
}

void MYSQL::lock()
{
	this->critical.lock();
	msqldriver->threadInit();
}

void MYSQL::unlock()
{
	this->critical.unlock();
	msqldriver->threadEnd();
}

void MYSQL::SetLastError(const char* in)
{
	FREE(lastError);

	const auto size = strlen(in);
	lastError = ALLOC<char>(size + 1);
	memcpy(lastError, in, size);
	lastError[size] = '\0';
}

char* MYSQL::GetLastError() const
{
	return lastError;
}

MYSQL_RESULT MYSQL::query(char* query, const bool retry)
{
	lock();

	if(retry)
		NET_LOG_WARNING(CSTRING("[MYSQL] - The previous query has been failed, proccessing the query again."));

	if (!msqldriver)
	{
		if (!retry)
		{
			if (reconnect())
			{
				unlock();
				return this->query(query, true);
			}
		}

		FREESTRING(query);
		NET_LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql driver instance does not exists"));
		unlock();
		return {};
	}

	if (!msqlcon)
	{
		if (!retry)
		{
			if (reconnect())
			{
				unlock();
				return this->query(query, true);
			}
		}

		FREESTRING(query);
		NET_LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql connection instance does not exists"));
		unlock();
		return {};
	}

	if (!msqlcon->isValid())
	{
		if (!retry)
		{
			if (reconnect())
			{
				unlock();
				return this->query(query, true);
			}
		}

		FREESTRING(query);
		NET_LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql connection is not valid"));
		unlock();
		return {};
	}

	try
	{
		auto* stmt = msqlcon->createStatement();
		auto* res = stmt->executeQuery(query);

		if (res && res->rowsCount() != 0)
			res->next();

		MYSQL_FREE(stmt);
		FREESTRING(query);
		if (res)
		{
			unlock();
			return MYSQL_RESULT(CSTRING(""), res);
		}
	}
	catch (sql::SQLException& e)
	{
		FREESTRING(query);

		SetLastError(e.what());

		if(memcmp(e.what(), CSTRING("No result available"), strlen(CSTRING("No result available"))) != 0)
		NET_LOG_ERROR(CSTRING("[MYSQL] - %s => {%i}"), e.what(), e.getErrorCode());
	}

	unlock();
	return {};
}

MYSQL_RESULT MYSQL::query(MYSQL_QUERY query, const bool retry)
{
	lock();

	if (retry)
		NET_LOG_WARNING(CSTRING("[MYSQL] - The previous query has been failed, proccessing the query again."));

	if (!msqldriver)
	{
		if (!retry)
		{
			if (reconnect())
			{
				unlock();
				return this->query(query, true);
			}
		}

		query.Free();
		NET_LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql driver instance does not exists"));
		unlock();
		return {};
	}

	if (!msqlcon)
	{
		if (!retry)
		{
			if (reconnect())
			{
				unlock();
				return this->query(query, true);
			}
		}

		query.Free();
		NET_LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql connection instance does not exists"));
		unlock();
		return {};
	}

	if (!msqlcon->isValid())
	{
		if (!retry)
		{
			if (reconnect())
			{
				unlock();
				return this->query(query, true);
			}
		}

		query.Free();
		NET_LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql connection is not valid"));
		unlock();
		return {};
	}

	try
	{
		auto* stmt = msqlcon->createStatement();
		auto* res = stmt->executeQuery(query.Get());

		if (res && res->rowsCount() != 0)
			res->next();

		MYSQL_FREE(stmt);
		query.Free();
		if (res)
		{
			unlock();
			return MYSQL_RESULT(CSTRING(""), res);
		}
	}
	catch (sql::SQLException& e)
	{
		query.Free();

		SetLastError(e.what());

		if (memcmp(e.what(), CSTRING("No result available"), strlen(CSTRING("No result available"))) != 0)
			NET_LOG_ERROR(CSTRING("[MYSQL] - %s => {%i}"), e.what(), e.getErrorCode());
	}

	unlock();
	return {};
}

MYSQL_MULTIRESULT MYSQL::multiQuery(MYSQL_MUTLIQUERY query, const bool retry)
{
	lock();

	if (retry)
		NET_LOG_WARNING(CSTRING("[MYSQL] - The previous query has been failed, proccessing the query again."));

	if (!msqldriver)
	{
		if (!retry)
		{
			if (reconnect())
			{
				unlock();
				return this->multiQuery(query, true);
			}
		}

		for (auto& curquery : query.Get())
			curquery.Free();

		NET_LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql driver instance does not exists"));
		unlock();
		return {};
	}

	if (!msqlcon)
	{
		if (!retry)
		{
			if (reconnect())
			{
				unlock();
				return this->multiQuery(query, true);
			}
		}

		for (auto& curquery : query.Get())
			curquery.Free();

		NET_LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql connection instance does not exists"));
		unlock();
		return {};
	}

	if (!msqlcon->isValid())
	{
		if (!retry)
		{
			if (reconnect())
			{
				unlock();
				return this->multiQuery(query, true);
			}
		}

		for (auto& curquery : query.Get())
			curquery.Free();

		NET_LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql connection is not valid"));
		unlock();
		return {};
	}

	try
	{
		MYSQL_MULTIRESULT results;
		auto* stmt = msqlcon->createStatement();
		for (auto& curquery : query.Get())
		{
			auto* res = stmt->executeQuery(curquery.Get());

			if (res && res->rowsCount() != 0)
				res->next();

			results.Add(MYSQL_RESULT(curquery.getName(), res));
			curquery.Free();
		}
		MYSQL_FREE(stmt);

		unlock();
		return results;
	}
	catch (sql::SQLException& e)
	{
		for (auto& curquery : query.Get())
			curquery.Free();

		SetLastError(e.what());

		if (memcmp(e.what(), CSTRING("No result available"), strlen(CSTRING("No result available"))) != 0)
			NET_LOG_ERROR(CSTRING("[MYSQL] - %s => {%i}"), e.what(), e.getErrorCode());
	}

	unlock();
	return {};
}
#endif
