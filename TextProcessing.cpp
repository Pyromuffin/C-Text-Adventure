#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include <SFML/Window.hpp>

#include "TextProcessing.h"
#include "SFML/Graphics/Font.hpp"


static char* s_textBuffer = nullptr;

static const int INITIAL_TEXT_BUFFER_SIZE = 10000;
static int s_currentBufferSize = INITIAL_TEXT_BUFFER_SIZE;
static int s_bufferIndex = 0;

void InitText()
{
	s_textBuffer = (char*) malloc(sizeof(char) * INITIAL_TEXT_BUFFER_SIZE);
}

char* GetTextBuffer()
{
	return s_textBuffer;
}

void DoubleTextBuffer()
{
	auto mem = malloc(sizeof(char) * s_currentBufferSize * 2);
	memcpy(mem, s_textBuffer, s_currentBufferSize);
	s_currentBufferSize *= 2;
	free(s_textBuffer);
	s_textBuffer = (char*)mem;
}

void Print(const char* formatString, ...)
{
	va_list argList;
	int remainingBuffer = s_currentBufferSize - s_bufferIndex;


	va_start(argList, formatString);
	int charsWritten = vsnprintf(&s_textBuffer[s_bufferIndex], remainingBuffer, formatString, argList);
	if (charsWritten >= remainingBuffer)
	{
		DoubleTextBuffer();
		charsWritten = vsnprintf(&s_textBuffer[s_bufferIndex], s_currentBufferSize - s_bufferIndex, formatString, argList);
	}
	s_bufferIndex += charsWritten;

	va_end(argList);
}


char GetCharFromKeycode(const sf::Keyboard::Key keycode)
{
	if (keycode >= sf::Keyboard::Key::A && keycode <= sf::Keyboard::Key::Z)
	{
		const auto keyOffset = keycode - sf::Keyboard::Key::A;
		const char key = 'a' + keyOffset;
		return key;
	}
	else if (keycode == sf::Keyboard::Key::Space)
	{
		return ' ';
	}

	return '\0';
}


bool HandleKeyDown(const sf::Event& e, CommandString& commandString)
{
	bool processCommand = false;
	const auto keycode = e.key.code;
	char c = GetCharFromKeycode(keycode);

	if (keycode == sf::Keyboard::Return)
	{
		processCommand = true;
	}
	else if (keycode == sf::Keyboard::BackSpace)
	{
		commandString.DeleteCharacterAtCursor();
		commandString.blinkTimer.ResetBlinkStatus();
	}
	else if (keycode == sf::Keyboard::Left)
	{
		if (e.key.control)
		{
			commandString.SeekLeftToNextWord();
		}
		else
		{
			commandString.MoveCursor(-1);
		}

		commandString.blinkTimer.ResetBlinkStatus();
	}
	else if (keycode == sf::Keyboard::Right)
	{
		if (e.key.control)
		{
			commandString.SeekRightToNextWord();
		}
		else
		{
			commandString.MoveCursor(1);
		}

		commandString.blinkTimer.ResetBlinkStatus();
	}
	else if (keycode == sf::Keyboard::Home)
	{
		commandString.SeekToBeginning();
		commandString.blinkTimer.ResetBlinkStatus();
	}
	else if (keycode == sf::Keyboard::End)
	{
		commandString.SeekToEnd();
		commandString.blinkTimer.ResetBlinkStatus();
	}
	else if (c != '\0')
	{
		if (e.key.shift && c >= 'a' && c <= 'z')
		{
			c -= 'a';
			c += 'A';
		}

		commandString.InsertCharacterAtCursor(c);
		commandString.blinkTimer.ResetBlinkStatus();
	}

	return processCommand;
}

/*
struct SurfingBoy
{
	SDL_Surface* surf;
	int newLines;
};

void GetLine(char* text, bool &done, int& nextStart, int& numberOfLines )
{
	// go until we find a \n or a \0
	numberOfLines = 0;
	done = false;
	nextStart = 0;
	bool eatingSpaces = false;
	int pos = 0;
	char c = text[pos];

	while (c != '\0')
	{
	
		if (c == '\n')
		{
			numberOfLines++;
			eatingSpaces = true;
		}
		else if(eatingSpaces)
		{
			return;
		}


		pos++;
		nextStart = pos;
		c = text[pos];
	}

	done = true;
	return;
}

bool NeedsWrapping(char* text, int windowWidth, int padding)
{
	int width, height;
	TTF_SizeText(g_font, text, &width, &height);
	return width >= windowWidth + padding * 2;
}


void RenderScreenText(char* commandString)
{
	static const SDL_Color white{ 255,255,255,255 };
	int yOffset = 75;
	int newLineSpace = 20;
	int lineSpacing = 0;
	int windowPadding = 15;
	int windowWidth, windowHeight;
	SDL_GetWindowSize(g_window, &windowWidth, &windowHeight);


	
	std::vector<SurfingBoy> surfs;

	int bufferPos = 0;
	while (bufferPos < s_bufferIndex)
	{
		int lineStart = bufferPos;
		bool done;
		int numberOfLines;
		int nextStart;

		GetLine(&s_textBuffer[bufferPos], done, nextStart, numberOfLines);
		
		if (!done) s_textBuffer[lineStart + nextStart -1] = '\0';
		
	

		auto surf = TTF_RenderText_Blended(g_font, &s_textBuffer[lineStart], white);
		
		if (!done) s_textBuffer[lineStart + nextStart -1] = '\n';

		SurfingBoy boy;
		boy.newLines = numberOfLines;
		if (done) boy.newLines++;
		boy.surf = surf;

		bufferPos = lineStart + nextStart;

		surfs.push_back(boy);
	}
	
 	

	for (int i = 0; i < surfs.size(); i++)
	{
		SurfingBoy surf = surfs[surfs.size() - i - 1];
		if (!surf.surf)
		{
			continue;
		}
		SDL_Rect srcRect{ 0,0, surf.surf->w, surf.surf->h };

		int yPos = windowHeight  - yOffset - newLineSpace * surf.newLines;
		yOffset += surf.surf->h + newLineSpace * (surf.newLines -1);
		SDL_Rect dstRect{ windowPadding, yPos, surf.surf->w, surf.surf->h };
		SDL_BlitSurface(surf.surf, &srcRect, g_backbuffer, &dstRect);
		SDL_FreeSurface(surf.surf);
	}
	



	


	// render >
	auto pointySurf = TTF_RenderText_Blended(g_font, ">", white);
	int pointyWidth = pointySurf->w;
	{
		SDL_Rect srcRect{ 0,0, pointySurf->w, pointySurf->h };
		SDL_Rect dstRect{ windowPadding, windowHeight - windowPadding - pointySurf->h, pointySurf->w, pointySurf->h };
		SDL_BlitSurface(pointySurf, &srcRect, g_backbuffer, &dstRect);
		SDL_FreeSurface(pointySurf);
	}
	

	// do command string at bottom
	if (commandString[0] == '\0')
	{
		return;
	}

	// kerning doesn't work so i guess we die.

	auto surf = TTF_RenderText_Blended(g_font, commandString, white);
	SDL_Rect srcRect{ 0,0, surf->w, surf->h };
	SDL_Rect dstRect{ windowPadding + pointyWidth + 5, windowHeight - windowPadding - surf->h, surf->w, surf->h };
	SDL_BlitSurface(surf, &srcRect, g_backbuffer, &dstRect);
	SDL_FreeSurface(surf);
}
*/

void CommandString::Reset()
{
	m_buffer[0] = '>';
	m_buffer[1] = ' ';
	m_buffer[2] = '\0';

	m_cursorPosition = cursorStartPosition;
	m_length = cursorStartPosition;
}

void CommandString::MoveCursor(const int amount)
{
	m_cursorPosition = std::clamp(m_cursorPosition + amount, cursorStartPosition, m_length);
}

void CommandString::SeekRightToNextWord()
{
	MoveCursor(1);

	while (m_cursorPosition < m_length)
	{
		const bool textUnderCursor = m_buffer[m_cursorPosition] != ' ';
		const bool spaceToTheLeft = m_buffer[m_cursorPosition - 1] == ' ';

		if (textUnderCursor && spaceToTheLeft)
			break;
		MoveCursor(1);
	}
}

void CommandString::SeekLeftToNextWord()
{
	// check that the value under the cursor is a character, and the value to the left is a space.
	MoveCursor(-1);

	while (m_cursorPosition > cursorStartPosition)
	{
		const bool textUnderCursor = m_buffer[m_cursorPosition] != ' ';
		const bool spaceToTheLeft = m_buffer[m_cursorPosition - 1] == ' ';
		
		if (textUnderCursor && spaceToTheLeft)
			break;
		MoveCursor(-1);
	}
}

void CommandString::SeekToBeginning()
{
	m_cursorPosition = cursorStartPosition;
}

void CommandString::SeekToEnd()
{
	m_cursorPosition = m_length;
}

void CommandString::InsertCharacterAtCursor(const char c)
{
	// copy string one character place down.
	for (int i = 0; i < m_length - m_cursorPosition + 1; i++)
	{
		const int index = m_length - i;
		m_buffer[index + 1] = m_buffer[index];
	}

	m_buffer[m_cursorPosition] = c;
	m_length++;
	MoveCursor(1);
}

void CommandString::DeleteCharacterAtCursor()
{
	if (m_cursorPosition == cursorStartPosition)
		return;

	for (int index = m_cursorPosition - 1; index < m_length; index++)
	{
		m_buffer[index] = m_buffer[index + 1];
	}

	m_length--;
	MoveCursor(-1);
}



int GetBufferStartOfVisibleText( sf::Window* window, sf::Font* font, const int fontSize)
{
	// first we need to know how many lines of text fit in the window.
	const float windowHeight = window->getSize().y;
	const float glyphHeight = font->getLineSpacing(fontSize);

	const int numberOfLinesInWindow = static_cast<int>(windowHeight / glyphHeight) - 1; // minus one for the command string
	// actually here are some dragons because 
	
	int foundNewlines = 0;
	int index;
	// now go back through the buffer until we find number of lines \n's
	for(index = s_bufferIndex; index >= 0; index--)
	{
		if( s_textBuffer[index] == '\n' )
		{
			foundNewlines++;
			if (foundNewlines == numberOfLinesInWindow)
			{
				index++; // go past that newline otherwise we'll just be printing a new line and nothing else at the top row.
				break;
			}
		}
	}
	
	return index;
}