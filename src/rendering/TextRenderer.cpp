#include <iostream>

#include "TextRenderer.h"
#include "Config.h"

bool TextRenderer::renderText( SDL_Renderer* grenderer, const char* cText )
{
    fsurface = TTF_RenderText_Solid( font, cText, fontColor );
    if ( fsurface == nullptr ) {
        std::cout << "Failed to create surface from the text " << cText << ", error: " << TTF_GetError() << '\n';
        return false;
    }

    ftexture = SDL_CreateTextureFromSurface( grenderer, fsurface );
    if ( ftexture == nullptr ) {
        std::cout << "Failed to create font texture from font surface, error: " << SDL_GetError() << '\n';
        return false;
    }

    textBox.x = 0;
    textBox.y = 0;
    textBox.w = fsurface->w;
    textBox.h = fsurface->h;
    SDL_RenderCopy( grenderer, ftexture, NULL, &textBox );
    SDL_DestroyTexture( ftexture );
    SDL_FreeSurface( fsurface );
    ftexture = NULL;
    fsurface = NULL;

    return true;
}