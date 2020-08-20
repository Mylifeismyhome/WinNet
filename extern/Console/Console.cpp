#include "Console.h"

static Console* mStaticInstance = nullptr;

static bool CommandsListPopup = false;
static size_t CommandsListPopupIndex = 0;
static UINT CommandsIndex = -1;
static UINT countVisibleCommands = 0;

Console::Console()
{
	m_id = 0;
	m_Buffer = std::string("");
	m_Pos = m_StartPos;
	m_EntriesToTop = 0;
	m_FontSize = 26.f;

	mStaticInstance = this;
}

Console::~Console()
{
	vBuffer.clear();
}

void Console::Insert(const char* buffer)
{
	auto tmp = sBuffer();
	tmp.id = m_id++;
	tmp.buffer = (const char*)_strdup(buffer);
	vBuffer.emplace_back(tmp);
}

void Console::Remove(DWORD id)
{
	for (std::vector<sBuffer>::size_type i = 0; i != vBuffer.size(); i++)
	{
		if (vBuffer[i].id == id)
		{
			vBuffer.erase(vBuffer.begin() + i);
			--m_id;
			break;
		}
	}
}

void Console::ClearVBuffer()
{
	vBuffer.clear();
	m_id = 0;
}

std::vector<Console::sBuffer> Console::GetVectorBuffer() const
{
	return vBuffer;
}

static const char* const KeyBoardLayout_small = "abcdefghijklmnopqrstuvwxyz";
static const char* const KeyBoardLayout_big = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char* const KeyBoardLayout_number = "0123456789";
static const char* const KeyBoardLayout_numberSpecial = "=!\"�$%&/()";

static float scroll;

static bool Capital = false;

static UINT tabIndex = 0;
static const char* LastCommandBuffer = "";

void Console::Update(const HWND hwnd)
{
	// Capital Lock
	if (GUI_KEYPRESSEDONCE(GUI_KEYS::CAPITAL))
		Capital = !Capital;

	// SHIFT
	if (Capital && !GUI_KEYISPRESSED(GUI_KEYS::SHIFT) || !Capital && GUI_KEYISPRESSED(GUI_KEYS::SHIFT))
	{
		DWORD key = 0x41; // start with A
		for (unsigned int i = 0; i < strlen(KeyBoardLayout_big); i++)
		{
			if (GUI_KEYPRESSEDONCE(key))
			{
				m_Buffer += KeyBoardLayout_big[i];
				CommandsListPopup = false;
				CommandsListPopupIndex = 0;
				countVisibleCommands = 0;
				LastCommandBuffer = "";
			}

			key++;
		}

		key = 0x30;
		for (unsigned int i = 0; i < strlen(KeyBoardLayout_numberSpecial); i++)
		{
			if (GUI_KEYPRESSEDONCE(key))
			{
				m_Buffer += KeyBoardLayout_numberSpecial[i];
				CommandsListPopup = false;
				CommandsListPopupIndex = 0;
				countVisibleCommands = 0;
				LastCommandBuffer = "";
			}

			key++;
		}
	}
	else
	{
		DWORD key = 0x41; // start with a
		for (unsigned int i = 0; i < strlen(KeyBoardLayout_small); i++)
		{
			if (GUI_KEYPRESSEDONCE(key))
			{
				m_Buffer += KeyBoardLayout_small[i];
				CommandsListPopup = false;
				CommandsListPopupIndex = 0;
				countVisibleCommands = 0;
				LastCommandBuffer = "";
			}

			key++;
		}

		key = 0x30;
		for (unsigned int i = 0; i < strlen(KeyBoardLayout_number); i++)
		{
			if (GUI_KEYPRESSEDONCE(key))
			{
				m_Buffer += KeyBoardLayout_number[i];
				CommandsListPopup = false;
				CommandsListPopupIndex = 0;
				countVisibleCommands = 0;
				LastCommandBuffer = "";
			}

			key++;
		}
	}

	// Space
	if (GUI_KEYPRESSEDONCE(0x20))
		m_Buffer += ' ';

	// Remove
	TIMER PressedRemove = CURRENTCLOCKTIME;
	static float speed = 0.1f;
	if (KEYISPRESSED(0x08))
	{
		if (PressedRemove < CURRENTCLOCKTIME)
		{
			if (m_Buffer.length() > 0)
			{
				m_Buffer.erase(m_Buffer.end() - 1);
				CommandsListPopup = false;
				CommandsListPopupIndex = 0;
				countVisibleCommands = 0;
				LastCommandBuffer = "";
			}

			PressedRemove = CREATETIMER(speed);
			speed *= 0.90f;
		}
	}
	else
		speed = 0.1f;


	// Clear
	if (GUI_KEYPRESSEDONCE(0x2E) || GUI_KEYPRESSEDONCE(0x1B)) // delete & escape
	{
		m_Buffer.clear();
		CommandsListPopup = false;
		CommandsListPopupIndex = 0;
		countVisibleCommands = 0;
		LastCommandBuffer = "";
	}

	if (GUI_KEYPRESSEDONCE(GUI_KEYS::F2))
		ClearVBuffer();

	// Execute
	if (GUI_KEYPRESSEDONCE(0x0D)) // Return
	{
		if (m_Buffer.length() > 0)
			ExecuteCommandLine();
	}

	// Scrolling
	if (GUI_MOUSEWHEEL(GUI_WHEEL::UP))
	{
		if (GetEntriesToTop() > 0)
			SetPos((GetPos() - GetFontSize()));
	}

	TIMER PressedScroller = CURRENTCLOCKTIME;
	static auto speedScroller = 0.1f;
	if (GUI_KEYISPRESSED(0x21)) // Page UP
	{
		if (PressedScroller < CURRENTCLOCKTIME)
		{
			if (GetEntriesToTop() > 0)
				SetPos((GetPos() - GetFontSize()));

			PressedScroller = CREATETIMER(speedScroller);
			speedScroller *= 0.90f;
		}
	}
	else
		speedScroller = 0.1f;

	if (GUI_MOUSEWHEEL(GUI_WHEEL::DOWN))
	{
		if (GetPos() < m_StartPos)
		{
			SetPos((GetPos() + GetFontSize()));
			if (GetPos() > m_StartPos)
				SetPos(m_StartPos);
		}
	}

	static float speedScroller2 = 0.1f;
	if (GUI_KEYISPRESSED(0x22)) // Page DOWN
	{
		if (PressedScroller < CURRENTCLOCKTIME)
		{
			if (GetPos() < m_StartPos)
				SetPos((GetPos() + GetFontSize()));

			PressedScroller = CREATETIMER(speedScroller2);
			speedScroller2 *= 0.90f;
		}
	}
	else
		speedScroller2 = 0.1f;

	// Switch Command
	static size_t lastUsedIndex = 0;
	if (GUI_KEYPRESSEDONCE(GUI_KEYS::UP))
	{
		if (CommandsListPopup)
		{
			CommandsListPopupIndex++;

			if (CommandsListPopupIndex > countVisibleCommands)
				CommandsListPopupIndex = 1;
		}
		else
		{
			if (!vLastUsedCommands.empty())
			{
				if (lastUsedIndex >= vLastUsedCommands.size())
					lastUsedIndex = 0;

				m_Buffer.clear();
				m_Buffer += '!';
				m_Buffer += vLastUsedCommands[lastUsedIndex++];
			}
		}
	}

	if (GUI_KEYPRESSEDONCE(GUI_KEYS::DOWN))
	{
		if (CommandsListPopup)
		{
			if (CommandsListPopupIndex >= 0)
			{
				CommandsListPopupIndex--;

				if (CommandsListPopupIndex <= 0)
					CommandsListPopupIndex = countVisibleCommands;
			}
		}
		else
		{
			if (!vLastUsedCommands.empty())
			{
				if (lastUsedIndex >= vLastUsedCommands.size())
					lastUsedIndex = vLastUsedCommands.size() - static_cast<size_t>(1);

				m_Buffer.clear();
				m_Buffer += '!';
				m_Buffer += vLastUsedCommands[lastUsedIndex--];
			}
		}
	}

	// Tab
	if (GUI_KEYPRESSEDONCE(GUI_KEYS::TAB))
	{
		if (!vCommands.empty())
		{
			auto similar = false;
			if (!m_Buffer.empty())
			{
				for (std::vector<Console::sCommands>::size_type i = 0; i != vCommands.size(); i++)
				{
					if (strcmp(LastCommandBuffer, vCommands[i].Command) == 0)
						continue;

					auto subBuffer = std::string(m_Buffer.begin() + 1, m_Buffer.end());
					if (std::string(vCommands[i].Command).find(subBuffer) != std::string::npos)
					{
						const char begin = m_Buffer[0];
						m_Buffer.clear();
						if (begin == '!')
							m_Buffer += '!';
						else if (begin == '/')
							m_Buffer += '/';
						else
							m_Buffer += '!';
						m_Buffer += vCommands[i].Command;

						LastCommandBuffer = vCommands[i].Command;
						similar = true;
						break;
					}
				}
			}

			if (!similar)
			{
				if (tabIndex >= vCommands.size())
					tabIndex = 0;

				const auto newBuffer = vCommands[tabIndex++].Command;

				if (!m_Buffer.empty())
				{
					const char begin = m_Buffer[0];
					m_Buffer.clear();
					if (begin == '!')
						m_Buffer += '!';
					else if (begin == '/')
						m_Buffer += '/';
					else
						m_Buffer += '!';
					m_Buffer += newBuffer;
					LastCommandBuffer = newBuffer;
				}
				else
				{
					m_Buffer.clear();
					m_Buffer += '!';
					m_Buffer += newBuffer;
					LastCommandBuffer = newBuffer;
				}
			}
		}
	}
}

void Console::Render(Renderer* context)
{
	const auto Yoffset = 48.0f;
	auto OffsetY = (static_cast<float>(context->GetLocalSize().height) - Yoffset) - GetPos();

	// Mask Start Pos
	const auto P1_MaskX = 0.0f, P1_MaskY = (static_cast<float>(context->GetLocalSize().height) - Yoffset);
	const auto P2_MaskX = static_cast<float>(context->GetLocalSize().width), P2_MaskY = GetFontSize();

	auto MaxPos = (static_cast<float>(context->GetLocalSize().height) - Yoffset);

	auto EntriesToTop = 0.0f;
	auto EntriesToDown = 0.0f;

	// Text
	const auto vBuffer = Net::Console::GetLogs();
	for (auto it = Net::Console::GetLogSize(); it > 0; --it)
	{
		if (!vBuffer[it])
			continue;

		if (OffsetY <= P2_MaskY)
			EntriesToTop++;

		if (OffsetY > P1_MaskY)
			EntriesToDown++;

		if (OffsetY <= P1_MaskY && OffsetY > P2_MaskY) // check inside mask
		{
			float height;

			if (vBuffer[it].buffer)
				height = context->PaintText(vBuffer[it].buffer, 10, OffsetY, GetOutColor(vBuffer[it].state), GetFontSize(), static_cast<float>(context->GetLocalSize().width) - 40.0f).height;
			else if (vBuffer[it].wbuffer)
				height = context->PaintText(vBuffer[it].wbuffer, 10, OffsetY, GetOutColor(vBuffer[it].state), GetFontSize(), static_cast<float>(context->GetLocalSize().width) - 40.0f).height;

			OffsetY -= height;
			MaxPos -= height;
		}
		else
			OffsetY -= GetFontSize();
	}

	// Scroller Max Pos
	SetEntriesToTop(static_cast<UINT>(EntriesToTop));

	if (EntriesToTop > 0 || EntriesToDown > 0)
	{
		// Background
		context->DrawRectangleFilled(static_cast<float>(context->GetLocalSize().width) - 4.0f, 48, static_cast<float>(context->GetLocalSize().width) - 14.0f, ((static_cast<float>(context->GetLocalSize().height) - Yoffset) + 27.0f), color::White, 1.0f);

		// Inside
		const float MaxHeight = (context->GetLocalSize().height - 18.0f);
		const float MinHeight = 50;

		const float PercentageMax = ((float)EntriesToTop / (float)MaxLines);
		float newMaxHeight = (MaxHeight - (MaxHeight * PercentageMax));
		newMaxHeight = CLAMP(newMaxHeight, MinHeight, MaxHeight);
		const float PercentageMin = ((float)EntriesToDown / (float)MaxLines);
		float newMinHeight = (MinHeight + (MaxHeight * PercentageMin));
		newMinHeight = CLAMP(newMinHeight, MinHeight, MaxHeight);
		context->DrawRectangleFilled(static_cast<float>(context->GetLocalSize().width) - 6.0f, newMinHeight, static_cast<float>(context->GetLocalSize().width) - 12.0f, newMaxHeight, color::Black, 1.0f);
	}

	// Input
	context->DrawRectangleFilled(85.0f, 0, static_cast<float>(context->GetLocalSize().width), 40, color::Gray);

	static auto InputStyle = false;
	TIMER SwapTimer = CURRENTCLOCKTIME;
	if (SwapTimer < CURRENTCLOCKTIME)
	{
		InputStyle = !InputStyle;
		SwapTimer = CREATETIMER(0.5);
	}

	static auto InputWidth = 0.0f;
	if (InputStyle)
	{
		const auto InputSize = strlen(GetBuffer());
		auto Input = ALLOC<char>(InputSize + 2);
		sprintf(Input, "%s|", GetBuffer());
		InputWidth = context->PaintText(Input, InputWidth < static_cast<float>(context->GetLocalSize().width) ? 90.0f : (static_cast<float>(context->GetLocalSize().width) - InputWidth), static_cast<float>(context->GetLocalSize().height) - 7.5f, color::White, 24, static_cast<float>(context->GetLocalSize().width) - 15.0f, 0, false).width;
		FREE(Input);
	}
	else
	{
		const auto InputSize = strlen(GetBuffer());
		auto Input = ALLOC<char>(InputSize + 1);
		sprintf(Input, "%s", GetBuffer());
		InputWidth = context->PaintText(Input, InputWidth < static_cast<float>(context->GetLocalSize().width) ? 90.0f : (static_cast<float>(context->GetLocalSize().width) - InputWidth), static_cast<float>(context->GetLocalSize().height) - 7.5f, color::White, 24, static_cast<float>(context->GetLocalSize().width) - 15.0f, 0, false).width;
		FREE(Input);
	}

	// Clock
	context->DrawRectangleFilled(0.0f, 0, 85.0f, 40, { 0.4f, 0.4f, 0.4f, 1.0f });

	char* time = nullptr;
	CURRENTTIME(time);
	const auto RealTimeSize = strlen(time);
	auto RealTime = ALLOC<char>(RealTimeSize + 3);
	memcpy(&RealTime[0], "[", 1);
	memcpy(&RealTime[1], time, RealTimeSize);
	memcpy(&RealTime[RealTimeSize + 1], "]", 1);
	RealTime[RealTimeSize + 2] = '\0';
	context->PaintText(RealTime, 10.0f, static_cast<float>(context->GetLocalSize().height) - 7.5f, color::White, 24, 90.0f, 0, false);
	FREE(time);
	FREE(RealTime);

	// show all commands
	if (m_Buffer.find('!') == 0 || m_Buffer.find('/') == 0)
	{
		CommandsListPopup = true;
		countVisibleCommands = 0;
		auto height = 40;
		size_t Index = 1;
		for (std::vector<Console::sCommands>::size_type i = 0; i != vCommands.size(); i++)
		{
			if (m_Buffer.find(vCommands[i].Command) != std::string::npos)
				break;

			std::string subBuffer = std::string(m_Buffer.begin() + 1, m_Buffer.end());
			if (std::string(vCommands[i].Command).find(subBuffer) != std::string::npos)
			{
				if (CommandsListPopupIndex > 0)
				{
					if (Index == CommandsListPopupIndex)
						context->DrawRectangleFilled(0, static_cast<float>(height), static_cast<float>(context->GetLocalSize().width), static_cast<float>(height + 40), color::LightBlue);
					else
						context->DrawRectangleFilled(0, static_cast<float>(height), static_cast<float>(context->GetLocalSize().width), static_cast<float>(height + 40), color::DarkGray);
				}
				else
					context->DrawRectangleFilled(0, static_cast<float>(height), static_cast<float>(context->GetLocalSize().width), static_cast<float>(height + 40), color::DarkGray);

				height += 40;
				countVisibleCommands++;
				Index++;
			}
		}

		height = 40;
		for (std::vector<Console::sCommands>::size_type i = 0; i != vCommands.size(); i++)
		{
			if (m_Buffer.find(vCommands[i].Command) != std::string::npos)
				break;

			std::string subBuffer = std::string(m_Buffer.begin() + 1, m_Buffer.end());
			if (std::string(vCommands[i].Command).find(subBuffer) != std::string::npos)
			{
				const auto commandSize = strlen(vCommands[i].Command);
				auto command = ALLOC<char>(commandSize + 2);
				sprintf(command, "%c%s", m_Buffer[0], vCommands[i].Command);
				context->PaintText(command, 10, static_cast<float>(context->GetLocalSize().height) - static_cast<float>(height) - 7.5f, color::White, 24, static_cast<float>(context->GetLocalSize().width), 0, false);
				FREE(command);
				height += 40;
			}
		}
	}
	else
	{
		CommandsListPopup = false;
		CommandsListPopupIndex = 0;
		countVisibleCommands = 0;
	}


	//DrawButton(context, 100, 100, 50, 20, &test2);
}

const char* Console::GetBuffer() const
{
	return m_Buffer.c_str();
}

void Console::SetPos(const float pos)
{
	m_Pos = pos;
}

const float Console::GetPos() const
{
	return m_Pos;
}

void Console::SetEntriesToTop(const UINT entries)
{
	m_EntriesToTop = entries;
}

const UINT Console::GetEntriesToTop() const
{
	return m_EntriesToTop;
}

void Console::SetFontSize(const float size)
{
	m_FontSize = size;
}

const float Console::GetFontSize() const
{
	return m_FontSize;
}

color Console::GetOutColor(CONSOLE::LogStates state) const
{
	switch (state)
	{
	case CONSOLE::LogStates::normal:
		return color::White;

	case CONSOLE::LogStates::debug:
		return color::LightBlue;

	case CONSOLE::LogStates::warning:
		return color::LightGoldenrodYellow;

	case CONSOLE::LogStates::error:
		return color::IndianRed;

	case CONSOLE::LogStates::success:
		return color::LightGreen;

	case CONSOLE::LogStates::peer:
		return color::LightYellow;

	default:
		return color::White;
	}
}

void ClearOutput()
{
	CONSOLE::ClearLogs();
}

void Console::RegisterCommands()
{
	REGISTER_COMMAND(this, "clear", ClearOutput);
	REGISTER_COMMAND(this, "cls", ClearOutput);
	REGISTER_COMMAND(this, "list used", ListAllLastUsedCommands);
}

void Console::ExecuteCommandLine()
{
	if (CommandsListPopup && CommandsListPopupIndex > 0)
	{
		int Index = 1;
		for (std::vector<Console::sCommands>::size_type i = 0; i != vCommands.size(); i++)
		{
			std::string subBuffer = std::string(m_Buffer.begin() + 1, m_Buffer.end());
			if (std::string(vCommands[i].Command).find(subBuffer) != std::string::npos)
			{
				if (Index == CommandsListPopupIndex)
				{
					char begin = m_Buffer[0];
					m_Buffer.clear();
					if (begin == '!')
						m_Buffer += '!';
					else if (begin == '/')
						m_Buffer += '/';
					else
						m_Buffer += '!';
					m_Buffer += vCommands[i].Command;
					break;
				}

				Index++;
			}
		}

		CommandsListPopup = false;
		CommandsListPopupIndex = 0;
		countVisibleCommands = 0;
		return;
	}

	const auto Slash = m_Buffer.find('/');
	const auto ExclamationMark = m_Buffer.find('!');
	if (Slash == std::string::npos && ExclamationMark == std::string::npos)
	{
		NLOG("Input does not comply with the specifications!");
		m_Buffer.clear();
		return;
	}

	// erase '/' or '!'
	if (Slash == std::string::npos)
		m_Buffer.erase(m_Buffer.begin() + ExclamationMark);
	else
		m_Buffer.erase(m_Buffer.begin() + Slash);

	bool found = false;
	for (std::vector<sCommands>::size_type i = 0; i != vCommands.size(); i++)
	{
		if (strcmp(vCommands[i].Command, m_Buffer.c_str()) == 0)
		{
			(*vCommands[i].func)();

			// Ignore List used
			if (strcmp(vCommands[i].Command, "list used") != 0)
			{
				for (std::vector<const char*>::size_type j = 0; j != vLastUsedCommands.size(); j++)
				{
					if (strcmp(vLastUsedCommands[j], vCommands[i].Command) == 0)
					{
						vLastUsedCommands.erase(vLastUsedCommands.begin() + j);
						break;
					}
				}
				vLastUsedCommands.emplace_back(vCommands[i].Command);
				std::reverse(vLastUsedCommands.begin(), vLastUsedCommands.end());
			}

			found = true;
			break;
		}
	}

	if (!found)
		NLOG("Command has not been found!");

	m_Buffer.clear();
}

void Console::ListAllLastUsedCommands()
{
	CLOG("---------- ALL LAST USED COMMANDS ----------");
	for (std::vector<const char*>::size_type i = (mStaticInstance->vLastUsedCommands.size() - 1); i != -1; i--)
		CLOG("%i: %s", (i + 1), mStaticInstance->vLastUsedCommands[i]);
	CLOG("----------------------------------------------------");
}

UINT Console::GetIndicatorCommands() const
{
	return CommandsIndex++;
}