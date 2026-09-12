#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

class TextRenderer
{
public:
    bool renderText( SDL_Renderer* grenderer, const char* cText );
    TTF_Font* font;

private:
    SDL_Surface* fsurface = nullptr;
    SDL_Texture* ftexture = nullptr;
    SDL_Rect textBox;
    SDL_Color fontColor = { 0xFF, 0x00, 0xFF, 0xFF };
};