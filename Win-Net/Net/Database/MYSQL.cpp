#include "MYSQL.h"

MYSQL_CON::MYSQL_CON()
{
	sprintf_s(conIP, CSTRING(""));
	conPort = 0;

	sprintf_s(conUsername, CSTRING(""));
	sprintf_s(conPassword, CSTRING(""));
	sprintf_s(conDB, CSTRING(""));
}

MYSQL_CON::MYSQL_CON(const char* ip, const short port, const char* username, const char* password, const char* db)
{
	sprintf_s(conIP, ip);
	conPort = port;

	sprintf_s(conUsername, username);
	sprintf_s(conPassword, password);
	sprintf_s(conDB, db);
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
	sprintf_s(name, CSTRING(""));
	result = nullptr;
	valid = false;
}

MYSQL_RESULT::MYSQL_RESULT(const char* n, sql::ResultSet* res)
{
	sprintf_s(name, n);
	result = res;
	valid = true;
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
	sprintf_s(name, n);
	query = q;
}

MYSQL_QUERY::MYSQL_QUERY(char* q)
{
	sprintf_s(name, CSTRING(""));
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

MYSQL_MUTLIQUERY::MYSQL_MUTLIQUERY(const std::vector<MYSQL_QUERY> q)
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

void MYSQL_MULTIRESULT::Add(MYSQL_RESULT res)
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
	FREE(lastError);
}

bool MYSQL::setup()
{
	if (msqldriver)
	{
		LOG_ERROR(CSTRING("[MYSQL] - Failure on setup, mysql driver instance exists"));
		return false;
	}

	msqldriver = get_driver_instance();
	return true;
}

bool MYSQL::connect()
{
	try
	{
		char constr[128];
		sprintf_s(constr, CSTRING("tcp://%s:%i"), conConfig.getIP(), conConfig.getPort());
		msqlcon = msqldriver->connect(constr, conConfig.getUsername(), conConfig.getPassword());
		msqlcon->setSchema(conConfig.getDB());
		return msqlcon->isValid();
	}
	catch (sql::SQLException& e)
	{
		SetLastError(e.what());
		LOG_ERROR(CSTRING("[MYSQL] - %s => {%i}"), e.what(), e.getErrorCode());
	}

	return false;
}

bool MYSQL::disconnect()
{
	try
	{
		msqlcon->close();

		const auto closed = msqlcon->isClosed();

		delete msqlcon;
		msqlcon = nullptr;

		return closed;
	}
	catch (sql::SQLException& e)
	{
		SetLastError(e.what());
		LOG_ERROR(CSTRING("[MYSQL] - %s => {%i}"), e.what(), e.getErrorCode());
	}

	return false;
}

bool MYSQL::reconnect()
{
	disconnect();
	const auto res = connect();
	if (!res)
		LOG_ERROR(CSTRING("[MYSQL] - Failure on re-estabilishing connection"));

	return res;
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

MYSQL_RESULT MYSQL::query(char* query, bool retry)
{
	if (!msqldriver)
	{
		if (!retry)
		{
			if (reconnect())
				return this->query(query, true);
		}

		LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql driver instance does not exists"));
		return {};
	}

	if (!msqlcon)
	{
		if (!retry)
		{
			if (reconnect())
				return this->query(query, true);
		}

		LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql connection instance does not exists"));
		return {};
	}

	if (!msqlcon->isValid())
	{
		if (!retry)
		{
			if (reconnect())
				return this->query(query, true);
		}

		LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql connection is not valid"));
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
			return MYSQL_RESULT(CSTRING(""), res);
	}
	catch (sql::SQLException& e)
	{
		if (!retry)
		{
			if (reconnect())
				return this->query(query, true);
		}

		SetLastError(e.what());

		if(memcmp(e.what(), CSTRING("No result available"), strlen(CSTRING("No result available"))) != 0)
		LOG_ERROR(CSTRING("[MYSQL] - %s => {%i}"), e.what(), e.getErrorCode());
	}

	return {};
}

MYSQL_RESULT MYSQL::query(MYSQL_QUERY query, bool retry)
{
	if (!msqldriver)
	{
		if (!retry)
		{
			if (reconnect())
				return this->query(query, true);
		}

		LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql driver instance does not exists"));
		return {};
	}

	if (!msqlcon)
	{
		if (!retry)
		{
			if (reconnect())
				return this->query(query, true);
		}

		LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql connection instance does not exists"));
		return {};
	}

	if (!msqlcon->isValid())
	{
		if (!retry)
		{
			if (reconnect())
				return this->query(query, true);
		}

		LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql connection is not valid"));
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
			return MYSQL_RESULT(CSTRING(""), res);
	}
	catch (sql::SQLException& e)
	{
		if (!retry)
		{
			if (reconnect())
				return this->query(query, true);
		}

		SetLastError(e.what());
	
		if (memcmp(e.what(), CSTRING("No result available"), strlen(CSTRING("No result available"))) != 0)
			LOG_ERROR(CSTRING("[MYSQL] - %s => {%i}"), e.what(), e.getErrorCode());
	}

	return {};
}

MYSQL_MULTIRESULT MYSQL::multiQuery(MYSQL_MUTLIQUERY query, bool retry)
{
	if (!msqldriver)
	{
		if (!retry)
		{
			if (reconnect())
				return this->multiQuery(query, true);
		}

		LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql driver instance does not exists"));
		return {};
	}

	if (!msqlcon)
	{
		if (!retry)
		{
			if (reconnect())
				return this->multiQuery(query, true);
		}

		LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql connection instance does not exists"));
		return {};
	}

	if (!msqlcon->isValid())
	{
		if (!retry)
		{
			if (reconnect())
				return this->multiQuery(query, true);
		}

		LOG_ERROR(CSTRING("[MYSQL] - Failure on query, mysql connection is not valid"));
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

		return results;
	}
	catch (sql::SQLException& e)
	{
		if (!retry)
		{
			if (reconnect())
				return this->multiQuery(query, true);
		}

		SetLastError(e.what());
		
		if (memcmp(e.what(), CSTRING("No result available"), strlen(CSTRING("No result available"))) != 0)
			LOG_ERROR(CSTRING("[MYSQL] - %s => {%i}"), e.what(), e.getErrorCode());
	}

	return {};
}