#include <cstdio>
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



static bool HandleEvents(CommandString& commandString)
{
	bool processCommand = false;
	sf::Event e;

	while (window->pollEvent(e))
	{
		switch (e.type)
		{
		case sf::Event::Closed:
			window->close();
			SetProgramRunningMode(ProgramRunningMode::kQuitting);
			break;
		case sf::Event::KeyPressed:
			processCommand = HandleKeyDown(e, commandString);
			break;
		}
	}

	return processCommand;
}



void RenderText(CommandString& commandString)
{
	static sf::Text cursor{ "|", font };

	static const float padding = 15.0f;
	static const int fontSize = 30;
	static const float glyphSpace = font.getGlyph(' ', fontSize, false).advance;
	static const float halfGlyph = glyphSpace / 2.0f;

	const auto windowSize = window->getSize();
	const auto lineSpace = font.getLineSpacing(fontSize);

	const int cursorPos = commandString.GetCursorPosition();

	char* allTheText = GetTextBuffer();
	const int visibleOffset = GetBufferStartOfVisibleText(window, &font, 30);
	char* windowedText = &allTheText[visibleOffset];

	sf::Text text = sf::Text(commandString.GetBuffer(), font);
	sf::Text previous{ windowedText, font };

	cursor.setPosition(padding + cursorPos * glyphSpace - halfGlyph, windowSize.y - lineSpace - padding);
	text.setPosition(padding, windowSize.y - lineSpace - padding);
	previous.setPosition(padding, 0);
	
	window->clear();
	window->draw(text);
	window->draw(previous);
	if (commandString.blinkTimer.GetBlinkStatus()) window->draw(cursor);
	window->display();
}

int main(int argc, char *argv[])
{
    Init();

	CommandString commandString;
    PrintArrivalGreeting(kDefaultRoom);

    while (GetProgramRunningMode() == kPlaying || GetProgramRunningMode() == kDead)
    {
		bool processCommand = HandleEvents(commandString);
	
		if (processCommand)
		{
			Print("\n%s\n", commandString.GetBuffer());

			TrimSelf(commandString.GetBuffer());
			SetDebugGlobals(commandString.GetBuffer());

			auto inputTokens = AllocateTokenString(commandString.GetBuffer());

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

			commandString.Reset();
		}

		RenderText(commandString);
    }

    CleanUp();
    return 0;
}
