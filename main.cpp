#include <stdio.h>
#include <ctime>
#include <iostream>

#include <SDL.h>
#include "SDL_TTF\include\SDL_ttf.h"

#include "room.h"
#include "parse.h"
#include "IndexVector.h"
#include "state.h"
#include "items.h"
#include "StringHash.h"
#include "GameState.h"

static SDL_Window* s_window;
static SDL_Renderer* s_renderer;
static TTF_Font* s_font;

#define RALEWAY_PATH "C:\\Users\\pyrom\\Documents\\GitHub\\C-Text-Adventure\\Raleway\\Raleway-Regular.ttf"

static void InitSDL()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	s_window = SDL_CreateWindow("C Text Adventure", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 500, 0);
	s_renderer = SDL_CreateRenderer(s_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	TTF_Init();
	s_font = TTF_OpenFont(RALEWAY_PATH, 24);
}

static void Init()
{
    InitVectorTracking();
    RegisterCommands();

    MakeRooms();
    MakeSomeItems();
    MakeDirectionReferents();
	InitSDL();
}

static void CleanUp()
{
	SDL_DestroyRenderer(s_renderer);
	SDL_DestroyWindow(s_window);
	TTF_CloseFont(s_font);
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


static void UpdateScreenText(char* commandString)
{
	SDL_Color white{ 255,255,255,255 };

	auto surf = TTF_RenderText_Blended(s_font, commandString, white);
	SDL_Rect srcRect{ 0,0, surf->w, surf->h };
	SDL_Rect dstRect{ 500, 250, surf->w, surf->h };
	auto texture = SDL_CreateTextureFromSurface(s_renderer, surf);

	SDL_RenderClear(s_renderer);
	SDL_RenderCopy(s_renderer, texture, &srcRect, &dstRect);
	SDL_RenderPresent(s_renderer);
}


static int backspaceRepeatDelay = 500;
static int backspaceRepeatRate = 200;

static void EventLoop(char* commandString)
{
	int bufferPos = 0;
	uint lastBackspace = 0;
	uint firstBackspace = 0;
	
	bool backspacing = false;
	bool repeatingBackspace = false;
	bool waitingForNextRepeat = false;

	SDL_Event e;
	bool waiting = true;
	while (waiting)
	{
		SDL_PollEvent(&e);
		if (e.type == SDL_QUIT)
		{
			SetProgramRunningMode(ProgramRunningMode::kQuitting);
		}

		if (e.type == SDL_KEYDOWN)
		{
			auto keycode = e.key.keysym.sym;

			if ( keycode == SDLK_RETURN)
			{
				waiting = false;
			}
			else if (keycode == SDLK_BACKSPACE)
			{
				if (backspacing)
				{
					repeatingBackspace = (e.key.timestamp - firstBackspace) > backspaceRepeatDelay;
					waitingForNextRepeat = repeatingBackspace && (e.key.timestamp - lastBackspace) > backspaceRepeatRate;

					if (!repeatingBackspace || waitingForNextRepeat)
					{
						continue;
					}
					else
					{
						lastBackspace =
					}
				}


				if (bufferPos > 1)
				{
					backspacing = true;
					lastBackspace = e.key.timestamp;
					commandString[--bufferPos] = '\0';
					UpdateScreenText(commandString);
				}
			}
			else if (keycode <= 'z' && keycode > 0)
			{
				char key = keycode;
				commandString[bufferPos++] = key;
				commandString[bufferPos] = '\0';
				UpdateScreenText(commandString);
			}
		}

		if (e.type == SDL_KEYUP)
		{
			auto keycode = e.key.keysym.sym;
			if (keycode == SDLK_BACKSPACE)
			{
				backspacing = false;
			}
		}
	}

	commandString[bufferPos] = '\0';
}


int main(int argc, char *argv[])
{
    Init();

    char commandString[256] = "Fake Command!";
    PrintArrivalGreeting(kDefaultRoom);

    while (GetProgramRunningMode() == kPlaying || GetProgramRunningMode() == kDead)
    {
        printf("\n> ");
        fflush(stdout);

		EventLoop(commandString);
		printf(commandString);

        if (commandString[0] == '\n')
            continue;

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
            printf("What's that !?\n");
        }
    }

    CleanUp();
    return 0;
}
