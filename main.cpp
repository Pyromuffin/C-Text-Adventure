#include <stdio.h>
#include <ctime>
#include <algorithm>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "room.h"
#include "parse.h"
#include "IndexVector.h"
#include "state.h"
#include "items.h"
#include "StringHash.h"
#include "GameState.h"
#include "TextProcessing.h"


static sf::RenderWindow* window; 
static sf::Font font;

#define RALEWAY_PATH "C:\\Users\\pyrom\\Documents\\GitHub\\C-Text-Adventure\\Raleway\\Raleway-Regular.ttf"
#define INCONSOLATA_PATH "C:\\Users\\pyrom\\Documents\\GitHub\\C-Text-Adventure\\Inconsolata\\Inconsolata-Regular.ttf"

static void InitSFML()
{
	window = new sf::RenderWindow(sf::VideoMode(1000, 600), "C-Text-Adventure"); // window cannot be statically constructed for reasons.
	font.loadFromFile(INCONSOLATA_PATH);
}

static void Init()
{
    InitVectorTracking();
    RegisterCommands();

    MakeRooms();
    MakeSomeItems();
    MakeDirectionReferents();
	InitText();

	InitSFML();
}

static void CleanUp()
{
	delete window;
    CheckForVectorLeaks();
}

static void SetDebugGlobals(char* commandString)
{
	size_t length = strlen(commandString);

	if ((commandString[length - 1] == '~') && (commandString[length - 2] == '~'))
	{
		g_RawDebugParse = true;
		commandString[length - 2] = '\0';
	}
	else if (commandString[length - 1] == '~')
	{
		g_DebugParse = true;
		commandString[length - 1] = '\0';
	}
}

static double secondsToWait = 0.5;

static void SpeedCheck(char *command, int bufSize)
{
	/*
	int bufPos = 0;

	while (!_kbhit())
	{
		int c = _getche();
		command[bufPos++] = c;

		auto seconds = difftime(time(nullptr), lastTime);
		if (seconds < secondsToWait)
		{
			_cputs(" slower ");
		}
		lastTime = time(nullptr);

		if (c == '\r' ||  c == '\n')
		{
			command[bufPos-1] = '\0';
			break;
		}
	}
	*/
}


char GetChar(sf::Keyboard::Key keycode)
{
	if (keycode >= sf::Keyboard::Key::A && keycode <= sf::Keyboard::Key::Z)
	{
		auto keyOffset = keycode - sf::Keyboard::Key::A;
		char key = 'a' + keyOffset;
		return key;
	}
	else if (keycode == sf::Keyboard::Key::Space)
	{
		return ' ';
	}

	return '\0';
}

static int s_commandStringPos;


class CursorBlinkTimer
{
	sf::Clock blinkTimer;
	bool showCursor = true;

public:
	int cursorBlinkMs = 500;

	bool GetBlinkStatus()
	{
		if (blinkTimer.getElapsedTime().asMilliseconds() >= cursorBlinkMs)
		{
			blinkTimer.restart();
			showCursor = !showCursor;
		}

		return showCursor;
	}

	void ResetBlinkStatus()
	{
		showCursor = true;
	}
};


static CursorBlinkTimer s_blinkTimer;

static bool HandleEvents(char* commandString)
{
	bool processCommand = false;
	sf::Event e;

	while (window->pollEvent(e))
	{
		if (e.type == sf::Event::Closed)
		{
			window->close();
			SetProgramRunningMode(ProgramRunningMode::kQuitting);
		}

		if (e.type == sf::Event::KeyPressed)
		{
			auto keycode = e.key.code;
			char c = GetChar(keycode);

			if (keycode == sf::Keyboard::Return)
			{
				processCommand = true;
			}
			else if (keycode == sf::Keyboard::BackSpace)
			{
				if (s_commandStringPos == 0)
					continue;

				int length = strlen(commandString);
				for (int i = s_commandStringPos -1; i < length; i++)
				{
					commandString[i] = commandString[i + 1];
				}

				s_commandStringPos = std::max(s_commandStringPos - 1, 0);
				s_blinkTimer.ResetBlinkStatus();
			}
			else if (keycode == sf::Keyboard::Left)
			{
				int amount = 1;

				if (e.key.control)
				{
					int index = s_commandStringPos -2;
					while (commandString[index--] != ' ' && index >= 0) amount++;

					if (index < 0) s_commandStringPos = 0;
				}

				s_commandStringPos = std::max(s_commandStringPos - amount, 0);
				
				s_blinkTimer.ResetBlinkStatus();
			}
			else if (keycode == sf::Keyboard::Right)
			{
				// test comment do not upvoate

				int amount = 1;
				int length = strlen(commandString);

				if (e.key.control)
				{
					int index = s_commandStringPos - 2;
					while (commandString[index++] != ' ' && index < length) amount++;

					if (index < 0) s_commandStringPos = 0;
				}

				s_commandStringPos = std::min((int)strlen(commandString), s_commandStringPos + amount);
				s_blinkTimer.ResetBlinkStatus();
			}
			else if (keycode == sf::Keyboard::Home)
			{
				s_commandStringPos = 0;
				s_blinkTimer.ResetBlinkStatus();
			}
			else if (keycode == sf::Keyboard::End)
			{
				s_commandStringPos = strlen(commandString);
				s_blinkTimer.ResetBlinkStatus();
			}
			else if (c != '\0')
			{
				if (e.key.shift && c >= 'a' && c <= 'z')
				{
					c -= 'a';
					c += 'A';
				}

				// copy string one character place down.
				int length = strlen(commandString);
				for (int i = 0; i < length - s_commandStringPos + 1; i++)
				{
					int index = length - i;
					commandString[index + 1] = commandString[index];
				}

				commandString[s_commandStringPos] = c;
				s_commandStringPos++;
				s_blinkTimer.ResetBlinkStatus();

			}
		}
	}

	return processCommand;
}



void RenderText(char* commandString)
{
	static sf::Text carrot{ ">", font };
	static sf::Text cursor{ "|", font };

	static const int padding = 15;
	static const int fontSize = 30;
	static const int glyphSpace = font.getGlyph(' ', fontSize, false).advance;

	float carrotKerning = font.getKerning('>', commandString[0], fontSize); // TODO KERNING DOESNT WORK FOR SOME REASON 
	carrotKerning = 5; // update: the reason is a nightmare hell of unfathomable ruin.
	float carrotWidth = font.getGlyph('>', fontSize, false).advance;
	auto windowSize = window->getSize();
	auto lineSpace = font.getLineSpacing(fontSize);

	sf::Text text = sf::Text(commandString, font);
	sf::Text previous{ GetTextBuffer(), font };


	carrot.setPosition(padding, windowSize.y - lineSpace - padding);
	cursor.setPosition(padding + (s_commandStringPos + 1) * glyphSpace, windowSize.y - lineSpace - padding);
	text.setPosition(padding + carrotKerning + carrotWidth, windowSize.y - lineSpace - padding);
	previous.setPosition(padding, 0);
	
	window->clear();
	window->draw(carrot);
	window->draw(text);
	window->draw(previous);
	if (s_blinkTimer.GetBlinkStatus()) window->draw(cursor);
	window->display();
}

int main(int argc, char *argv[])
{
    Init();

    char commandString[256] = "";
    PrintArrivalGreeting(kDefaultRoom);

    while (GetProgramRunningMode() == kPlaying || GetProgramRunningMode() == kDead)
    {
		bool processCommand = HandleEvents(commandString);
	
		if (processCommand)
		{
			Print("\n>%s\n", commandString);

			TrimSelf(commandString);
			SetDebugGlobals(commandString);

			auto inputTokens = AllocateTokenString(commandString);

			ParseResult result = ParseInputString(inputTokens.get());
			fflush(stdout);

			if (result.valid)
			{
				const Command *command = GetCommand(result.commandLabel);
				command->execFunction(command, result.subject, result.object);
				fflush(stdout);
			}
			else
			{
				Print("What's that !?\n");
			}

			commandString[0] = '\0';
			s_commandStringPos = 0;
		}

		RenderText(commandString);
    }

    CleanUp();
    return 0;
}
