#pragma once

#include <SDL2/SDL.h>
#include <iostream>


namespace Config
{
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 960;

    constexpr int TEXTURE_WIDTH = 64;
    constexpr int TEXTURE_HEIGHT = 64;

    constexpr float MOVE_SPEED = 0.03;
    constexpr float ROT_SPEED = 0.02;

    constexpr float FOG_MAX_DISTANCE = 25.0f;
    constexpr Uint32 FOG_COLOR = 0xFF8000000;
}

class FileSystem
{
public:
    static std::string getAssetPath( const std::string& relativePath ) {
        static std::string basePath;

        if ( basePath.empty() ) {
            char* base = SDL_GetBasePath();
            if ( base ) {
                basePath = base;
                SDL_free( base );
            } else {
                std::cout << "Failed to get the base path, error: " << SDL_GetError() << '\n';
                basePath = "./";
            }
        }

        return basePath + "../assets/" + relativePath;
    }
};