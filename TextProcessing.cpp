#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>
#include <vector>

#include <SDL.h>
#include "SDL_TTF\include\SDL_ttf.h"

#include "TextProcessing.h"

extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;
extern SDL_Surface* g_backbuffer;
extern TTF_Font* g_font;

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

void Print(char* formatString,  ...)
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


void RenderScreenText(char* commandString)
{
	static const SDL_Color white{ 255,255,255,255 };
	/*
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
		
		auto surf = TTF_RenderText_Blended_Wrapped(g_font, &s_textBuffer[lineStart], white);
		
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
	*/



	int yOffset = 75;
	int newLineSpace = 20;
	int lineSpacing = 0;
	int windowPadding = 15;
	int windowWidth, windowHeight;
	SDL_GetWindowSize(g_window, &windowWidth, &windowHeight);

	{
		auto surf = TTF_RenderText_Blended_Wrapped(g_font, s_textBuffer, white, windowWidth - windowPadding * 2);
		SDL_Rect srcRect{ 0,0, surf->w, surf->h };
		SDL_Rect dstRect{ windowPadding, windowHeight - windowPadding - surf->h, surf->w, surf->h };
		SDL_BlitSurface(surf, &srcRect, g_backbuffer, &dstRect);
		SDL_FreeSurface(surf);
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

	auto surf = TTF_RenderText_Blended(g_font, commandString, white);
	SDL_Rect srcRect{ 0,0, surf->w, surf->h };
	SDL_Rect dstRect{ windowPadding + pointyWidth, windowHeight - windowPadding - surf->h, surf->w, surf->h };
	SDL_BlitSurface(surf, &srcRect, g_backbuffer, &dstRect);
	SDL_FreeSurface(surf);
}