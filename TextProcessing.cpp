#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#include <string>
#include <vector>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "STB/stb_truetype.h"

#include "utility.h"
#include "GameRender.h"
#include "TextProcessing.h"


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

int FindLineEndIndex(char* start)
{
	int index = 0;
	while (start[index] != '\0' && start[index] != '\n')
	{
		index++;
	}

	return index;
}

int FindLastWordBoundary(const char* text, int lineLength)
{
	int index = lineLength;
	while (text[index] != ' ' && index >= 0)
	{

		index--;
	}

	return index;
}


std::vector<std::string> SplitByLines(char* text)
{
	std::vector<std::string> lines;

	int index = 0;
	bool done = false;
	int lineStart = 0;


	while (!done)
	{

		index = FindLineEndIndex(&text[lineStart]);

		if (text[lineStart + index] == '\0')
			done = true;

		int count = index;
		auto string = std::string(&text[lineStart], count);
		lines.push_back(string);
		
		lineStart = lineStart + index + 1;
	}

	return lines;
}

std::vector<std::string> DoWordWrappingOnBlockOfText(sf::Window* window, sf::Font* font, const int fontSize, char *text, float padding)
{
	std::vector<std::string> lines = SplitByLines(text);

	// so i guess go through each "line" and find out the length of that line and then if that line exceeds the window length, then insert a \n at the word boundary before the line would break.
	float windowWidth = window->getSize().x - padding * 2; // TODO account for padding
	float glyphWidth = font->getGlyph(' ', fontSize, false).advance;
	int maxNumberOfCharsInLine = static_cast<int>(windowWidth / glyphWidth);

	for (int i = 0; i < lines.size(); i++)
	{
		auto& str = lines[i];

		if (str.length() > maxNumberOfCharsInLine)
		{
			// find last word index, and split!
			auto cString = str.c_str();
			int wordBoundary = FindLastWordBoundary(cString, maxNumberOfCharsInLine); // possibility of garbage lines if there are no word boundaries
			assert(wordBoundary != 0);
			std::string newString = std::string(&cString[wordBoundary + 1], str.length() - wordBoundary);
			str.resize(wordBoundary);
			lines.insert(lines.begin() + i + 1, newString);
		}

	}

	return lines;
}

int GetBufferStartOfVisibleText( sf::Window* window, sf::Font* font, const int fontSize, const int windowScrollOffset)
{
	// first we need to know how many lines of text fit in the window.
	const float windowHeight = window->getSize().y;
	const float glyphHeight = font->getLineSpacing(fontSize);

	const int numberOfLinesInWindow = static_cast<int>(windowHeight / glyphHeight) - 1; // minus one for the command string
	// actually here are some dragons because of the index change when we break out.
	
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


//font rendering

//stbtt_PackFontRange(stbtt_pack_context *spc, const unsigned char *fontdata, int font_index, float font_size,
// int first_unicode_char_in_range, int num_chars_in_range, stbtt_packedchar *chardata_for_range);



static unsigned char fontBuffer[1 << 20];

void AsciiArt(unsigned char* bitmap, int w, int h)
{
	for (int j = 0; j < h; ++j) {
		for (int i = 0; i < w; ++i)
			putchar(" .:ioVM@"[bitmap[j*w + i] >> 5]);
		putchar('\n');
	}
}



void InitFont(const char* path, FontData& info)
{
	constexpr int fontSize = 30;

	auto fontFile = fopen(path, "rb");
	fread(fontBuffer, 1, 1 << 20, fontFile);
	fclose(fontFile);

	stbtt_pack_context spc;
	stbtt_PackBegin(&spc, (unsigned char*)info.bitmap, info.x, info.y, 0, 1, nullptr);
	stbtt_PackSetOversampling(&spc, 2, 2);


	stbtt_PackFontRange(&spc, fontBuffer, 0, -fontSize, ' ', printableCharCount, info.packedChars);
	stbtt_PackEnd(&spc);
}

std::array<Vert, 6> QuadToVerts(const stbtt_aligned_quad q)
{
	std::array<Vert, 6> verts;
	// clockwise winding!
	// 0 1       4
    // 2      3  5

	verts[0] = { q.x0, q.y0, q.s0, q.t0 };
	verts[1] = { q.x1, q.y0, q.s1, q.t0 };
	verts[2] = { q.x0, q.y1, q.s0, q.t1 };
	verts[3] = verts[2]; // { q.x, q.y0, q.s0, q.t0 };
	verts[4] = verts[1]; // { q.x0, q.y0, q.s0, q.t0 };
	verts[5] = { q.x1, q.y1, q.s1, q.t1 };

	return verts;
}


void GetVerts(Vert* verts, char* str, float xPos, float yPos, FontData& info)
{
	int charCount = strlen(str);

	for (int i = 0; i < charCount; i++)
	{
		stbtt_aligned_quad quad;
		stbtt_GetPackedQuad(info.packedChars, info.x, info.y, str[i] - ' ', &xPos, &yPos, &quad, false);
		auto vertArray = QuadToVerts(quad);
		memcpy(verts + (i * 6), vertArray.data(), sizeof(vertArray));
	}
}


