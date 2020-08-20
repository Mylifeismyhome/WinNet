#pragma once
#include <Net/Net.h>
#include <assets/assets.h>

#include <stdio.h>
#include <string>
#include <vector>

#include <DirectX/include/Window.h>
#include <DirectX/include/Renderer.h>

#include <DirectX/include/Keyboard.h>
#include <DirectX/include/Mouse.h>

#include <DirectX/include/Button.hpp>

template <typename T> T CLAMP(const T& value, const T& low, const T& high)
{
	return value < low ? low : (value > high ? high : value);
}

#define REGISTER_COMMAND(c, name, pFunc) \
{ \
Console::sCommands s; \
s.index = c->GetIndicatorCommands(); \
s.Command = name; \
s.func = &pFunc; \
c->vCommands.emplace_back(s); \
NLOG_DEBUG("Registered Command %s", name); \
}

class Console
{
public:
	struct sBuffer
	{
		DWORD id;
		const char* buffer;
	};

	static const UINT MaxLines = 2500;

	struct sCommands
	{
		UINT index;
		const char* Command;
		void (*func)();
	};
	std::vector<sCommands> vCommands;

private:
	std::vector<sBuffer> vBuffer;

	DWORD m_id;
	std::string m_Buffer;

	const float m_StartPos = 0.0f;
	float m_Pos;

	UINT m_EntriesToTop;

	float m_FontSize;

	color GetOutColor(CONSOLE::LogStates) const;

	void ExecuteCommandLine();

	std::vector<const char*> vLastUsedCommands;

	static void ListAllLastUsedCommands();

public:
	Console();
	~Console();

	void Insert(const char*);
	void Remove(DWORD);
	void ClearVBuffer();
	std::vector<sBuffer> GetVectorBuffer() const;

	void Update(HWND);
	void Render(Renderer*);

	const char* GetBuffer() const;

	void SetPos(const float);
	const float GetPos() const;

	void SetEntriesToTop(const UINT);
	const UINT GetEntriesToTop() const;

	void SetFontSize(const float);
	const float GetFontSize() const;

	void RegisterCommands();
	UINT GetIndicatorCommands() const;
};
