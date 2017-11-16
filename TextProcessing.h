#pragma once

struct SDL_Texture;

void InitText();
void Print(char* formatString, ...);
char* GetTextBuffer();
void RenderScreenText(char* commandString);
