#pragma once


#include <SDL2/SDL.h>

#include "world/WorldObjects.h"
#include "world/Map.h"
#include "player/Player.h"
#include "Config.h"
#include "TextureManager.h"

class Renderer
{
public:
    bool init( SDL_Window* gwindow );
    void close();

    void renderClear();
    void renderFlush();

    void renderWall( const Player& player, const Wall& wall, const Sector& sector );

    void renderFrame( const Player& player, const Map& map, const TextureManager& TextureManager );
    

    SDL_Renderer* getRenderer();
    std::vector<Uint32> getBuffer();

private:
    void renderSector( const Player& player, const Sector& sector );
    void renderPortal( const Player& player, const Map& map, const Wall& wall, const Sector& currentSector );

    void renderFloor( const Player& player, const Sector& sector );
    void renderCeil( const Player& player, const Sector& sector );

    float normalizeAngle( float angle );
    Vx worldPosToCamera( const Player& player, Vx worldPos );
    int screenAngleToX( float angle );

    Vx intersectSegs( Vx a0, Vx a1, Vx b0, Vx b1 );

    void drawVerLine( int x, int yStart, int yEnd, Uint32 color );
    void drawPixel( int x, int y, Uint32 color );
    
    SDL_Renderer* grenderer = nullptr;
    SDL_Texture* gtexture = nullptr;

    std::vector<Uint32> buffer;

    // Occlusion tracking arrays for floor and ceiling
    int y_hi[ Config::SCREEN_WIDTH ];
    int y_lo[ Config::SCREEN_WIDTH ];

    struct RenderQueueItem {
        int sectorID;
        // Horizontal screen boundaries
        int x0, x1;
    };

    std::vector<RenderQueueItem> renderQueue;

    struct RayHit {
        bool hit;
        float distance;
        float wallPos;
    };

    RayHit checkRayIntersection( Vx rayOrigin, Vx rayDirection, Vx wallStart, Vx wallEnd );
};