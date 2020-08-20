#include "Client.h"

static BYTE* JuliansJSONString;
struct JuliansJSONRef
{
	char* str;
	double time;
	bool valid;

	JuliansJSONRef()
	{
		this->str = nullptr;
		this->time = NULL;
		this->valid = false;
	}

	JuliansJSONRef(char* pointer, double time)
	{
		this->str = pointer;
		this->time = time;
		this->valid = true;
	}
};
static JuliansJSONRef GetRndJulian()
{
	Net::Package json;
	if (!json.Parse(reinterpret_cast<const char*>(JuliansJSONString)))
		return JuliansJSONRef();

	auto arr = json.Array(CSTRING("Arr"));
	if (!arr.valid())
		return JuliansJSONRef();

	const auto obj = arr.value()[rand() % arr.value().Size()].GetObjectA();
	Net::Package jsonOBJ;
	jsonOBJ.SetPackage(obj);

	const auto val = jsonOBJ.String(CSTRING("Key"));
	if (!val.valid())
		return JuliansJSONRef();

	const auto time = jsonOBJ.Double(CSTRING("Timer"));
	if(!time.valid())
		return JuliansJSONRef();

	const auto str = ALLOC<char>(strlen(val.value()) + 1);
	memcpy(str, val.value(), strlen(val.value()));
	str[strlen(val.value())] = '\0';
	return JuliansJSONRef(str, time.value());
}

static bool LoadJulainsJSON()
{
	NET_FILEMANAGER fmanager(CSTRING("julians.json"), NET_FILE_BINARY | NET_FILE_READ);

	BYTE* data = nullptr;
	size_t size = -1;
	fmanager.read(data, size);
	JuliansJSONString = data;
	return true;
}

static bool ReloadJuliansJSON()
{
	FREE(JuliansJSONString);
	return LoadJulainsJSON();
}

Client::Client()
{
}

NET_CLIENT_BEGIN_DATA_PACKAGE(Client)
NET_CLIENT_DEFINE_PACKAGE(Test, Packages::PKG_TEST)
NET_CLIENT_END_DATA_PACKAGE

static bool estabilished = false;
void Client::Tick()
{
	if (estabilished)
	{
		if (KEYWASPRESSED(KEYBOARD::NUMPAD1))
		{
			if (ReloadJuliansJSON())
				LOG_SUCCESS(CSTRING("Julians JSON has been reloaded!"));
		}

		TIMER tt = -1;
		if (tt < CURRENTCLOCKTIME)
		{
			auto val = GetRndJulian();

			Package pkg;
			pkg.Append<const char*>(CSTRING("Thor"), val.str);
			DoSend(Packages::PKG_JulianStinkt, pkg);

			FREE(val.str);

			tt = CREATETIMER(val.time);
		}
	}
}

void Client::OnConnected()
{
}

void Client::OnConnectionEstabilished()
{
	LoadJulainsJSON();
	estabilished = true;
}

void Client::OnDisconnected()
{
}

void Client::OnForcedDisconnect(int)
{
}

void Client::OnKeysFailed()
{
}

void Client::OnKeysReceived()
{
}

void Client::OnTimeout()
{
}

void Client::OnVersionMismatch()
{
}

void Client::OnTest(NET_PACKAGE pkg)
{
	LOG("OK");
	const auto file = pkg.RawData(CSTRING("FILE"));
	if (!file.valid())
		return;

	const NET_FILEMANAGER fmanager(CSTRING("test2.mp4"), NET_FILE_BINARY | NET_FILE_WRITE | NET_FILE_DISCARD);
	fmanager.write(file.value(), file.size());
}