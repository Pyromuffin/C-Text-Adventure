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

static void InitSFML()
{
	window = new sf::RenderWindow(sf::VideoMode(1000, 600), "C-Text-Adventure"); // window cannot be statically constructed for reasons.
	font.loadFromFile(RALEWAY_PATH);
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


static bool HandleEvents(char* commandString)
{
	static int bufferPos = 0;
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
				commandString[--bufferPos] = '\0';
				bufferPos = std::max(bufferPos, 0);
			}
			else if (c != '\0')
			{
				if (e.key.shift && c >= 'a' && c <= 'z')
				{
					c -= 'a';
					c += 'A';
				}
				commandString[bufferPos++] = c;
				commandString[bufferPos] = '\0';
			}
		}
	}

	commandString[bufferPos] = '\0';
	return processCommand;
}


void RenderText(char* commandString)
{
	static sf::Text carrot{ ">", font };
	static const int padding = 15;
	static const int fontSize = 30;

	float carrotKerning = font.getKerning('>', commandString[0], fontSize); // TODO KERNING DOESNT WORK FOR SOME REASAON
	carrotKerning += 5;
	float carrotWidth = font.getGlyph('>', fontSize, false).advance;
	auto windowSize = window->getSize();
	auto lineSpace = font.getLineSpacing(fontSize);

	sf::Text text = sf::Text(commandString, font);

	carrot.setPosition(padding, windowSize.y - lineSpace - padding);
	text.setPosition(padding + carrotKerning + carrotWidth, windowSize.y - lineSpace - padding);

	sf::Text previous{ GetTextBuffer(), font };

	window->clear();
	window->draw(carrot);
	window->draw(text);
	window->draw(previous);
	window->display();
}

int main(int argc, char *argv[])
{
    Init();

    char commandString[256] = "Fake Command!";
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
		}

		RenderText(commandString);
    }

    CleanUp();
    return 0;
}
