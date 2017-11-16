#include <stdio.h>
#include <ctime>
#include <algorithm>

#include <SDL.h>
#include "SDL_TTF\include\SDL_ttf.h"

#include "room.h"
#include "parse.h"
#include "IndexVector.h"
#include "state.h"
#include "items.h"
#include "StringHash.h"
#include "GameState.h"
#include "TextProcessing.h"

SDL_Window* g_window;
SDL_Renderer* g_renderer;
TTF_Font* g_font;
SDL_Surface* g_backbuffer;

#define RALEWAY_PATH "C:\\Users\\pyrom\\Documents\\GitHub\\C-Text-Adventure\\Raleway\\Raleway-Regular.ttf"

static void InitSDL()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	g_window = SDL_CreateWindow("C Text Adventure", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 500, 0);
	g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	TTF_Init();
	g_font = TTF_OpenFont(RALEWAY_PATH, 24);
	g_backbuffer = SDL_DuplicateSurface(SDL_GetWindowSurface(g_window));
}

static void Init()
{
    InitVectorTracking();
    RegisterCommands();

    MakeRooms();
    MakeSomeItems();
    MakeDirectionReferents();
	InitSDL();
	InitText();
}

static void CleanUp()
{
	SDL_DestroyRenderer(g_renderer);
	SDL_DestroyWindow(g_window);
	TTF_CloseFont(g_font);
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


void ClearBackbuffer()
{
	SDL_LockSurface(g_backbuffer);
	memset(g_backbuffer->pixels, 0, g_backbuffer->format->BytesPerPixel  * g_backbuffer->w * g_backbuffer->h);
	SDL_UnlockSurface(g_backbuffer);
}

void PresentBackbuffer()
{
	auto tex = SDL_CreateTextureFromSurface(g_renderer, g_backbuffer);
	SDL_RenderCopy(g_renderer, tex, nullptr, nullptr);
	SDL_RenderPresent(g_renderer);
	SDL_DestroyTexture(tex);
}


static void UpdateScreenText(char* commandString)
{
	ClearBackbuffer();
	RenderScreenText(commandString);
	PresentBackbuffer();
}


static void EventLoop(char* commandString)
{
	int bufferPos = 0;

	SDL_Event e;
	bool waiting = true;
	while (waiting)
	{
		SDL_PollEvent(&e);
		if (e.type == SDL_QUIT)
		{
			SetProgramRunningMode(ProgramRunningMode::kQuitting);
			waiting = false;
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
				commandString[--bufferPos] = '\0';
				bufferPos = std::max(bufferPos, 0);
				UpdateScreenText(commandString);
			}
			else if (keycode <= 'z' && keycode >= ' ')
			{
				char key = keycode;
				if (e.key.keysym.mod & KMOD_SHIFT != 0 && key >= 'a' && key <= 'z')
				{
					key -= 'a';
					key += 'A';
				}
				commandString[bufferPos++] = key;
				commandString[bufferPos] = '\0';
				UpdateScreenText(commandString);
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
		EventLoop(commandString);
		Print("\n> %s\n", commandString);
		
	
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
            Print("What's that !?\n");
        }

		ClearBackbuffer();
		RenderScreenText("");
		PresentBackbuffer();
    }

    CleanUp();
    return 0;
}
