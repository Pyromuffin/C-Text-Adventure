#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

class CursorBlinkTimer
{
	sf::Clock m_blinkTimer;
	bool m_showCursor = true;

public:
	int cursorBlinkMs = 500;

	bool GetBlinkStatus()
	{
		if (m_blinkTimer.getElapsedTime().asMilliseconds() >= cursorBlinkMs)
		{
			m_blinkTimer.restart();
			m_showCursor = !m_showCursor;
		}

		return m_showCursor;
	}

	void ResetBlinkStatus()
	{
		m_showCursor = true;
	}
};


class CommandString
{
	static const int commandStringBufferSize = 256;
	static const int cursorStartPosition = 2;

	int m_length = cursorStartPosition;
	int m_cursorPosition = cursorStartPosition;
	char m_buffer[commandStringBufferSize] =  "> ";

public:
	CursorBlinkTimer blinkTimer;

	void Reset();
	void MoveCursor(int amount);
	void SeekRightToNextWord();
	void SeekLeftToNextWord();
	void SeekToBeginning();
	void SeekToEnd();
	void InsertCharacterAtCursor(char c);
	void DeleteCharacterAtCursor();
	char* GetBuffer() { return m_buffer; }
	int GetCursorPosition() const { return m_cursorPosition; }

};


void InitText();
void Print(const char* formatString, ...);
char* GetTextBuffer();

bool HandleKeyDown(const sf::Event& e, CommandString& commandString);

int GetBufferStartOfVisibleText(sf::Window* window, sf::Font* font, int fontSize, int windowScrollOffset);
std::vector<std::string> DoWordWrappingOnBlockOfText(sf::Window* window, sf::Font* font, const int fontSize, char *text, float padding);
