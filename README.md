main.cpp
Game
Renderer
Raycaster
Texture
World
Player
Input


now check both the header and the cpp for the game. especially if we are passing the right things into the functions:

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "../player/Player.h"
#include "../world/Map.h"
#include "../rendering/Renderer.h"
#include "../world/WorldObjects.h"

class Game
{
public:
    bool init();
    void close();

    void updatePlayer( Player& player, Map& map );
    bool checkLineIntersection( Vx k0, Vx k1, Vx l0, Vx l1 );
    bool checkPlayerWallCollision( const Player& player, Vx targetPos, const Map& map );

private:
    SDL_Window* gwindow = nullptr;

    Player player;
    Map map;
    Renderer renderer;

    bool running = false;
};

-----------------------

#include "Game.h"
#include "../Config.h"
#include <iostream>

bool Game::init()
{
    bool success = true;

    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        std::cout << "Failed to initialize sdl. Error: " << SDL_GetError() << '\n';
        success = false;
    } else {
        gwindow = SDL_CreateWindow( "Agmey's FPS", SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED, Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, 0 );
        if ( gwindow == NULL ) {
            std::cout << "Failed to create window. Error: " << SDL_GetError() << '\n';
            success = false;
        }

        int imgFlags = IMG_INIT_PNG;
        if ( !( IMG_Init( imgFlags ) & imgFlags ) ) {
            std::cout << "Failed to initialize image subsystem. Error: " << IMG_GetError() << '\n';
            success = false;
        }

        if ( TTF_Init() < 0 ) {
                std::cout << "Failed to initialize TTF. Error: " << TTF_GetError() << '\n';
                success = false;
            }
    }

    return success;
}

void Game::close()
{
    SDL_DestroyWindow( gwindow );
    gwindow = NULL;

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool checkLineIntersection( Vx k0, Vx k1, Vx l0, Vx l1 )
{
    // Returns 1 if the lines intersect, 0 otherwise

    float denom = ( ( k0.x - k1.x ) * ( l0.y - l1.y ) - ( k0.y - k1.y ) * ( l0.x - l1.x ) );
    constexpr float EPSILON = 1e-6f;

    if ( std::abs( denom ) < EPSILON )
        return false;

    float tNum = ( ( k0.x - l0.x ) * (l0.y - l1.y ) - ( k0.y - l0.y ) * ( l0.x - l1.x ) );
    float uNum = ( ( k0.y - k1.y ) * ( k0.x - l0.x ) - ( k0.x - k1.x ) * ( k0.y - l0.y ) );
    
    float t = tNum / denom;
    float u = uNum / denom;

    if ( 0.0f <= t && t <= 1.0f && 0.0f <= u && u <= 1.0f ) return true;
    return false;
}

bool checkPlayerWallCollision( const Player& player, Vx targetPos, const Map& map )
{
    Vx initialPos = { player.posX, player.posY };

    const Sector& sector = map.sectors[player.sector];

    for ( size_t i = sector.firstWall; i < sector.firstWall + sector.wallCount; ++i )
    {
        const Wall& wall = map.walls[i];

        if ( checkLineIntersection( initialPos, targetPos, wall.a, wall.b ) && wall.portal == -1 ) return true;
    }

    return false;
}

void Game::updatePlayer( Player& player, Map& map )
{
    const Uint8* keyState = SDL_GetKeyboardState( NULL );

    // Move forward if no wall
    if ( keyState[ SDL_SCANCODE_W ] ) {
        Vx targetPosX = { player.posX + player.dirX * Config::MOVE_SPEED, player.posY };
        if ( checkPlayerWallCollision( player, targetPosX, map ) == false ) player.posX += player.dirX * Config::MOVE_SPEED;
        Vx targetPosY = { player.posX, player.posY + player.dirY * Config::MOVE_SPEED };
        if ( checkPlayerWallCollision( player, targetPosY, map ) ) player.posY += player.dirY * Config::MOVE_SPEED;
    }
    // Move backwards if no wall
    if ( keyState[ SDL_SCANCODE_S ] ) {
        Vx targetPosX = { player.posX - player.dirX * Config::MOVE_SPEED, player.posY };
        if ( checkPlayerWallCollision( player, targetPosX, map ) == false ) player.posX -= player.dirX *Config::MOVE_SPEED;
        Vx targetPosY = { player.posX, player.posY - player.dirY * Config::MOVE_SPEED };
        if ( checkPlayerWallCollision( player, targetPosY, map ) == false ) player.posY -= player.dirY * Config::MOVE_SPEED;
    }
    // Rotate right
    if ( keyState[ SDL_SCANCODE_D ] ) {
        float oldDirX = player.dirX;
        player.dirX = player.dirX * cos( -Config::ROT_SPEED ) - player.dirY * sin( -Config::ROT_SPEED );
        player.dirY = oldDirX * sin( -Config::ROT_SPEED ) + player.dirY * cos( -Config::ROT_SPEED );
        float oldPlaneX = player.planeX;
        player.planeX = player.planeX * cos( -Config::ROT_SPEED ) - player.planeY * sin( -Config::ROT_SPEED );
        player.planeY = oldPlaneX * sin( -Config::ROT_SPEED ) + player.planeY * cos( -Config::ROT_SPEED );
    }
    // Rotate left
    if ( keyState[ SDL_SCANCODE_A ] ) {
        float oldDirX = player.dirX;
        player.dirX = player.dirX * cos( Config::ROT_SPEED ) - player.dirY * sin( Config::ROT_SPEED );
        player.dirY = oldDirX * sin( Config::ROT_SPEED ) + player.dirY * cos( Config::ROT_SPEED );
        float oldPlaneX = player.planeX;
        player.planeX = player.planeX * cos( Config::ROT_SPEED ) - player.planeY * sin( Config::ROT_SPEED );
        player.planeY = oldPlaneX * sin( Config::ROT_SPEED ) + player.planeY * cos( Config::ROT_SPEED );
    }
}