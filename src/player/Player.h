#pragma once


#include <SDL2/SDL.h>

#include "world/Map.h"

class Player
{
public:
    void updatePlayer( const Map& map, const Uint8* keyState, float frameTime );
    void checkPlayerWallCol( Vx currentPos, Vx& targetPos, const Map& map );
    void updatePlayerSector( const Map& map );

    float posX = 0.0;
    float posY = 0.0;
    float posZ = 0.8;

    float dirX = 0;
    float dirY = 1;
/*
    float planeX = 0.66;
    float planeY = 0;
*/
    float angle = 0.0;

    // One less due to indexing
    int sector = 0;

    bool airborne = false;
};