#include <cstdio>
#include <algorithm>

#include "utility.h"
#include "room.h"
#include "parse.h"
#include "IndexVector.h"
#include "state.h"
#include "items.h"
#include "StringHash.h"
#include "GameState.h"
#include "TextProcessing.h"
#include "ShaderCompiler.h"

#include <vulkan/vulkan.h>
#include "Vulkan.h"





static sf::Window* window; 
static sf::Font font;

#define RALEWAY_PATH "C:\\Users\\pyrom\\Documents\\GitHub\\C-Text-Adventure\\Raleway\\Raleway-Regular.ttf"
#define INCONSOLATA_PATH "C:\\Users\\pyrom\\Documents\\GitHub\\C-Text-Adventure\\Inconsolata\\Inconsolata-Regular.ttf"
#define VERTEX_SHADER_PATH "C:\\Users\\pyrom\\Documents\\GitHub\\C-Text-Adventure\\Shaders\\vertex.glsl"
#define FRAGMENT_SHADER_PATH "C:\\Users\\pyrom\\Documents\\GitHub\\C-Text-Adventure\\Shaders\\fragment.glsl"

GLuint shaderProgram;

static void InitSFML()
{
	
	//sf::ContextSettings contextSettings{ 24, 8, 0, 4, 6, sf::ContextSettings::Core | sf::ContextSettings::Debug, false };
	//sf::ContextSettings contextSettings{ 24, 8, 0, 4, 6, sf::ContextSettings::Default | sf::ContextSettings::Debug, false };

	window = new sf::Window(sf::VideoMode(1000, 600), "C-Text-Adventure", sf::Style::Default);


	/*
	font.loadFromFile(INCONSOLATA_PATH);
	
	auto vs = CompileShader(VERTEX_SHADER_PATH, GL_VERTEX_SHADER);
	auto fs = CompileShader(FRAGMENT_SHADER_PATH, GL_FRAGMENT_SHADER);

	shaderProgram = LinkShaders(vs, fs);


	glDeleteShader(vs);
	glDeleteShader(fs);
	*/
}

void error_callback(int error, const char* description)
{
	printf("Error: %s\n", description);
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
	InitVulkan(window);
	//InitFontAtlas(RALEWAY_PATH, window);
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

static int windowScrollOffset = 0;

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
		case sf::Event::MouseWheelScrolled:
			windowScrollOffset += e.mouseWheelScroll.delta;
		case sf::Event::Resized:
			sf::FloatRect visibleArea(0, 0, e.size.width, e.size.height);
			//window->setView(sf::View(visibleArea));
			break;
		}
	}
	
	return processCommand;
}



void RenderText(CommandString& commandString)
{
	/*
	static sf::Text cursor{ "|", font };

	static const float padding = 15.0f;
	static const int fontSize = 30;
	static const float glyphSpace = font.getGlyph(' ', fontSize, false).advance;
	static const float halfGlyph = glyphSpace / 2.0f;

	const auto windowSize = window->getSize();
	const auto lineSpace = font.getLineSpacing(fontSize);

	const int cursorPos = commandString.GetCursorPosition();

	char* allTheText = GetTextBuffer();
	const int visibleOffset = GetBufferStartOfVisibleText(window, &font, 30, windowScrollOffset);
	char* windowedText = &allTheText[visibleOffset];
	auto lines = DoWordWrappingOnBlockOfText(window, &font, 30, windowedText, padding);

	sf::Text text = sf::Text(commandString.GetBuffer(), font);
	
	cursor.setPosition(padding + cursorPos * glyphSpace - halfGlyph, windowSize.y - lineSpace - padding);
	text.setPosition(padding, windowSize.y - lineSpace - padding);
	
	//window->clear();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, window->getSize().x, window->getSize().y);

	//window->pushGLStates();
	DrawHelloWorld( window, shaderProgram);
	//window->popGLStates();


	//window->draw(text);

	float linePosition = windowSize.y - lineSpace * 2 - padding;
	int lineCount = 0;

	for (auto& line : reverse(lines))
	{
		auto drawLine = sf::Text{ line, font };
		drawLine.setPosition(padding, linePosition - lineSpace * lineCount);
	//	window->draw(drawLine);
		lineCount++;
	}

	//if (commandString.blinkTimer.GetBlinkStatus()) window->draw(cursor);

	window->display();
	*/
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
