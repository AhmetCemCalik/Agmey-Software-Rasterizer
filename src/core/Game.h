#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "player/Player.h"
#include "world/Map.h"
#include "rendering/Renderer.h"
#include "rendering/Texture.h"
#include "rendering/TextRenderer.h"
#include "rendering/TextureManager.h"
#include "world/WorldObjects.h"

class Game
{
public:
    bool init();
    void close();
    
    void run();
    void updateGame();
    void render();


    void updatePlayer( Player& player, Map& map, const Uint8* keyState );
    bool checkLineIntersection( Vx k0, Vx k1, Vx l0, Vx l1 );
    bool checkPlayerWallCollision( const Player& player, Vx targetPos, const Map& map );

private:
    SDL_Window* gwindow = nullptr;

    Player player;
    Map map;
    Renderer renderer;
    Texture texture;
    TextRenderer textRenderer;
    TextureManager textureManager;


    bool running = false;
};