#include <iostream>
#include <cmath>
#include <algorithm>

#include "Renderer.h"

// Standard geometric constants for our FOV calculations
constexpr float PI = 3.14159265359f;
constexpr float PI_2 = PI / 2.0f;
constexpr float PI_4 = PI / 4.0f;
constexpr float TAU = 2.0f * PI;
constexpr float HFOV = PI / 2.0f; // 90-degree Horizontal Field of View

// Wraps the angle between between -PI and +PI
float Renderer::normalizeAngle( float a ) 
{
    return a - ( TAU * std::floor( ( a + PI ) / TAU ) );
}

// Translates a world coordinate relative to the player, and rotates it by the player's viewing angle
Vx Renderer::worldPosToCamera( const Player& player, Vx worldPos ) {
    float u_x = worldPos.x - player.posX;
    float u_y = worldPos.y - player.posY;

    float cosA = std::cos( player.angle );
    float sinA = std::sin( player.angle );

    Vx camPos;
    // Project the world vector onto the camera's local Forward and Right axes
    camPos.x = u_x * cosA - u_y * sinA;   // Horizontal (Right) axis
    camPos.y = u_x * sinA + u_y * cosA;   // Depth (Forward) axis

    return camPos;
}

// 3. Projects a 2D angle relative to the camera onto a specific 1D pixel column on your monitor
int Renderer::screenAngleToX( float angle ) 
{
    return static_cast<int>( Config::SCREEN_WIDTH / 2.0f ) * 
           ( 1.0f - tan( ( ( angle + ( HFOV / 2.0f ) ) / HFOV ) * PI_2 - PI_4 ) );
}

Vx Renderer::intersectSegs( Vx a0, Vx a1, Vx b0, Vx b1 ) 
{
    float d = ( a0.x - a1.x ) * ( b0.y - b1.y ) - ( a0.y - a1.y ) * ( b0.x - b1.x );
    
    // If d is 0, the lines are parallel and will never intersect
    constexpr float EPSILON = 1e-6f;
    if ( std::abs( d ) < EPSILON ) return { NAN, NAN };

    float t = ( ( a0.x - b0.x ) * ( b0.y - b1.y ) - ( a0.y - b0.y ) * ( b0.x - b1.x ) ) / d;
    float u = ( ( a0.x - b0.x ) * ( a0.y - a1.y ) - ( a0.y - b0.y ) * ( a0.x - a1.x ) ) / d;

    // Check if the intersection happens strictly within the bounds of both segments
    if ( t >= 0 && t <= 1 && u >= 0 ) {
        return { a0.x + t * ( a1.x - a0.x ), a0.y + t * ( a1.y - a0.y ) };
    }
    
    return { NAN, NAN };
}

void Renderer::drawVerLine( int x, int yStart, int yEnd, Uint32 color )
{
    if ( x < 0 || x >= Config::SCREEN_WIDTH ) return;

    if ( yStart > yEnd ) std::swap( yStart, yEnd );

    yStart = std::clamp( yStart, 0, Config::SCREEN_HEIGHT - 1 );
    yEnd   = std::clamp( yEnd,   0, Config::SCREEN_HEIGHT - 1 );

    for ( int y = yStart; y <= yEnd; ++y )
    {
        buffer[ y * Config::SCREEN_WIDTH + x ] = color;
    }
}

void Renderer::drawPixel( int x, int y, Uint32 color )
{
    if ( x < 0 || x >= Config::SCREEN_WIDTH || y < 0 || y > Config::SCREEN_HEIGHT ) return;

    buffer[ y * Config::SCREEN_WIDTH + x ] = color;
}

inline Uint32 applyLighting( Uint32 color, float lightLevel ) 
{
    // ARGB
    Uint32 a = color & 0xFF000000;
    Uint32 r = ( color & 0x00FF0000 ) >> 16;
    Uint32 g = ( color & 0x0000FF00 ) >> 8;
    Uint32 b = color & 0x000000FF;

    r = static_cast<Uint32>( r * lightLevel );
    g = static_cast<Uint32>( g * lightLevel );
    b = static_cast<Uint32>( b * lightLevel );

    return a | ( r << 16 ) | ( g << 8 ) | b;
}

inline Uint32 applyFog( Uint32 color, float depth ) 
{
    float f = depth / Config::FOG_MAX_DISTANCE;
    if ( f > 1.0f ) f = 1.0f;
    if ( f < 0.0f ) f = 0.0f; 

    float invF = 1.0f - f; // The remaining percentage of the original color

    // Extract texture channels
    Uint32 a = color & 0xFF000000;
    Uint32 r1 = ( color & 0x00FF0000 ) >> 16;
    Uint32 g1 = ( color & 0x0000FF00 ) >> 8;
    Uint32 b1 = ( color & 0x000000FF );

    // Extract fog channels
    Uint32 r2 = ( Config::FOG_COLOR & 0x00FF0000 ) >> 16;
    Uint32 g2 = ( Config::FOG_COLOR & 0x0000FF00 ) >> 8;
    Uint32 b2 = ( Config::FOG_COLOR & 0x000000FF );

    // Lerp each channel independently
    Uint32 r = static_cast<Uint32>( r1 * invF + r2 * f );
    Uint32 g = static_cast<Uint32>( g1 * invF + g2 * f );
    Uint32 b = static_cast<Uint32>( b1 * invF + b2 * f );

    // Repack and return
    return a | ( r << 16 ) | ( g << 8 ) | b;
}

inline Uint32 applyLightEffects( Uint32 color, float lightLevel, float depth ) 
{
    // ARGB
    Uint32 a = color & 0xFF000000;
    Uint32 r = ( color & 0x00FF0000 ) >> 16;
    Uint32 g = ( color & 0x0000FF00 ) >> 8;
    Uint32 b = color & 0x000000FF;

    r = static_cast<Uint32>( r * lightLevel );
    g = static_cast<Uint32>( g * lightLevel );
    b = static_cast<Uint32>( b * lightLevel );

    color = a | ( r << 16 ) | ( g << 8 ) | b;

    float f = depth / Config::FOG_MAX_DISTANCE;
    if ( f > 1.0f ) f = 1.0f;
    if ( f < 0.0f ) f = 0.0f; 

    float invF = 1.0f - f; // The remaining percentage of the original color

    // Extract texture channels
    Uint32 a = color & 0xFF000000;
    Uint32 r1 = ( color & 0x00FF0000 ) >> 16;
    Uint32 g1 = ( color & 0x0000FF00 ) >> 8;
    Uint32 b1 = ( color & 0x000000FF );

    // Extract fog channels
    Uint32 r2 = ( Config::FOG_COLOR & 0x00FF0000 ) >> 16;
    Uint32 g2 = ( Config::FOG_COLOR & 0x0000FF00 ) >> 8;
    Uint32 b2 = ( Config::FOG_COLOR & 0x000000FF );

    // Lerp each channel independently
    Uint32 r = static_cast<Uint32>( r1 * invF + r2 * f );
    Uint32 g = static_cast<Uint32>( g1 * invF + g2 * f );
    Uint32 b = static_cast<Uint32>( b1 * invF + b2 * f );

    // Repack and return
    return a | ( r << 16 ) | ( g << 8 ) | b;
}

/*
void Renderer::renderFrame( const Player& player, const Map& map )
{
    // Seed the queue
    renderQueue.clear();
    renderQueue.push_back( { player.sector, 0, Config::SCREEN_WIDTH - 1 } );

    // Track drawn sectors
    std::vector<bool> sectorDrawn( map.getSectors().size(), false );

    constexpr float ZNEAR = 0.0001f;
    constexpr float ZFAR = 128.0f;

    // Left and right bounding rays
    // 1. Start the rays at exactly 0.0
    Vx znl = { 0.0f, 0.0f };
    Vx znr = { 0.0f, 0.0f };
    Vx zfl = { -std::sin( HFOV / 2.0f ) * ZFAR, std::cos( HFOV / 2.0f ) * ZFAR };
    Vx zfr = {  std::sin( HFOV / 2.0f ) * ZFAR, std::cos( HFOV / 2.0f ) * ZFAR };

    while ( !renderQueue.empty() ) {
        RenderQueueItem entry = renderQueue.back();
        renderQueue.pop_back();

        if ( sectorDrawn[ entry.sectorID ] ) continue;
        sectorDrawn[ entry.sectorID ] = true;

        const Sector& sector = map.getSectors()[ entry.sectorID ];
        const auto& walls = map.getWalls();

        std::cout << "Rendering Sector: " << entry.sectorID << " with " << sector.wallCount << " walls.\n";

        for ( size_t i = sector.firstWall; i < sector.firstWall + sector.wallCount; i++ ) {
            const Wall& wall = walls[ i ];

            // Translate to camera space
            Vx camPos0 = worldPosToCamera( player, wall.a );
            Vx camPos1 = worldPosToCamera( player, wall.b );

            // Disregard behind the player
            if ( camPos0.y <= 0 && camPos1.y <= 0 ) continue;

            // Angles of the two vertices
            float anglePos0 = normalizeAngle( std::atan2( camPos0.y, camPos0.x ) - PI_2 );
            float anglePos1 = normalizeAngle( std::atan2( camPos1.y, camPos1.x ) - PI_2 );

            // Frustum clipping
            // If wall clips the camera plane, disregard it
            if ( camPos0.y < ZNEAR || camPos1.y < ZNEAR || anglePos0 > +( HFOV / 2.0f ) || anglePos1 < -( HFOV / 2.0f ) ) {
                Vx il = intersectSegs( camPos0, camPos1, znl, zfl );
                Vx ir = intersectSegs( camPos0, camPos1, znr, zfr );

                if ( !std::isnan( il.x ) ) {
                    camPos0 = il;
                    anglePos0 = normalizeAngle( std::atan2( camPos0.y, camPos0.x ) - PI_2 );
                }
                if ( !std::isnan( ir.x ) ) {
                    camPos1 = ir;
                    anglePos1 = normalizeAngle( std::atan2( camPos1.y, camPos1.x ) - PI_2 );
                }
            }

            std::cout << "Wall " << i << " Angles: ap0=" << anglePos0 << " ap1=" << anglePos1 << "\n";
            
            // Backface culling
            if ( anglePos0 < anglePos1 ) continue;

            // Further backface culling to prevent screen wrapping glitches
            if ( ( anglePos0 < -( HFOV / 2 ) && anglePos1 < -( HFOV / 2 ) ) 
                || ( anglePos0 > +( HFOV / 2 ) && anglePos1 > +( HFOV / 2 ) ) ) continue;
            
            // Map the angles to screen pixel columns
            int tx0 = screenAngleToX( anglePos0 );
            int tx1 = screenAngleToX( anglePos1 );

            if ( tx0 > tx1 ) continue;

            // Portal boundary check
            if ( tx0 > entry.x1 ) continue;
            if ( tx1 < entry.x0 ) continue;

            // Clamp the rendering window bounds
            int x0 = std::max( tx0, entry.x0 );
            int x1 = std::min( tx1, entry.x1 );

            bool isPortal = ( wall.portal > 0 );
            int targetSector = isPortal ? ( wall.portal - 1 ) : -1;

            // Floor and ceiling heights
            float zfloor = sector.zfloor;
            float zceil = sector.zceil;
            float nzfloor = isPortal ? map.getSectors()[ targetSector ].zfloor : 0;
            float nzceil = isPortal ? map.getSectors()[ targetSector ].zceil : 0;

            // Perspective scale factors
            float sy0 = ( 0.5f * Config::SCREEN_HEIGHT ) / std::max( camPos0.y, 0.0001f );
            float sy1 = ( 0.5f * Config::SCREEN_HEIGHT ) / std::max( camPos1.y, 0.0001f );
            int halfH = Config::SCREEN_HEIGHT / 2;

            // Calculate exact Screen Y pixels for the current sector
            // Because screen Y goes down, higher Z values subtract from halfH (moving up the screen)
            int yf0 = halfH + static_cast<int>( ( player.posZ - nzfloor ) * sy0 );
            int yc0 = halfH - static_cast<int>( ( nzceil - player.posZ ) * sy0 );
            int yf1 = halfH + static_cast<int>( ( player.posZ - nzfloor ) * sy1 );
            int yc1 = halfH - static_cast<int>( ( nzceil - player.posZ ) * sy1 );

            // Calculate exact Screen Y pixels for the neighbor sector (if looking through portal)
            int nyf0 = halfH + static_cast<int>( ( player.posZ - nzfloor ) * sy0 );
            int nyc0 = halfH - static_cast<int>( ( nzceil - player.posZ ) * sy0 );
            int nyf1 = halfH + static_cast<int>( ( player.posZ - nzfloor ) * sy1 );
            int nyc1 = halfH - static_cast<int>( ( nzceil - player.posZ ) * sy1 );

            int txd = tx1 - tx0;
            if ( txd == 0 ) txd = 1;

            std::cout << "Drawing Wall " << i << " from x0=" << x0 << " to x1=" << x1 << "\n";
            // Loop across screen columns
            for ( int x = x0; x <= x1; x++ ) {
                float xp = static_cast<float>( x - tx0 ) / txd;

                // Interpolate x coords for this y pos
                int tyf = yf0 + static_cast<int>( xp * ( yf1 - yf0 ) );
                int tyc = yc0 + static_cast<int>( xp * ( yc1 - yc0 ) );

                // Clamp against occlusion buffers to not draw over foreground walls
                int yf = std::clamp( tyf, y_lo[ x ], y_hi[ x ] );
                int yc = std::clamp( tyc, y_lo[ x ], y_hi[ x ] );

                // Draw ceiling
                drawVerLine( x, y_lo[ x ], yc, 0xFF323232 );
                // Draw floor
                drawVerLine( x, yf, y_hi[ x ], 0xFF646464 );

                if ( isPortal ) {
                    int tnyf = nyf0 + static_cast<int>( xp * ( nyf1 - nyf0 ) );
                    int tnyc = nyc0 + static_cast<int>( xp * ( nyc1 - nyc0 ) );

                    int nyf = std::clamp( tnyf, y_lo[ x ], y_hi[ x ] );
                    int nyc = std::clamp( tnyc, y_lo[ x ], y_hi[ x ] );

                    // Draw Upper Step (If neighbor ceiling is lower, draw the wall connecting them)
                    drawVerLine( x, yc, nyc, 0xFFFF0000 ); // Red Step
                    
                    // Draw Lower Step (If neighbor floor is higher, draw the wall connecting them)
                    drawVerLine( x, nyf, yf, 0xFF0000FF ); // Blue Step

                    // Shrink the occlusion buffers! The next sector can ONLY draw inside this hole.
                    y_lo[ x ] = std::clamp( std::max( yc, nyc ), 0, Config::SCREEN_HEIGHT - 1 );
                    y_hi[ x ] = std::clamp( std::min( yf, nyf ), 0, Config::SCREEN_HEIGHT - 1 );
                } else {
                    // Solid Wall
                    drawVerLine( x, yc, yf, 0xFFFFFFFF ); // White Wall
                }
            }
            // Queue the connected sector to be rendered next
            if ( isPortal ) {
                renderQueue.push_back({ targetSector, x0, x1 });
            }
        }
    }
}
*/

void Renderer::renderFrame( const Player& player, const Map& map, const TextureManager& textureManager )
{
    // 1. THE QUEUE
    renderQueue.clear();
    renderQueue.push_back( { player.sector, 0, Config::SCREEN_WIDTH - 1 } );

    std::vector<bool> sectorDrawn( map.getSectors().size(), false );

    while ( !renderQueue.empty() ) {
        RenderQueueItem entry = renderQueue.back();
        renderQueue.pop_back();

        if ( sectorDrawn[ entry.sectorID ] ) continue;
        sectorDrawn[ entry.sectorID ] = true;

        const Sector& sector = map.getSectors()[ entry.sectorID ];
        
        for ( size_t i = sector.firstWall; i < sector.firstWall + sector.wallCount; i++ ) {
            const Wall& wall = map.getWalls()[ i ];

            // 2. TRANSFORM TO CAMERA SPACE
            Vx p0 = worldPosToCamera( player, wall.a );
            Vx p1 = worldPosToCamera( player, wall.b );

            // Fetch the texture
            const Texture& currentTex = textureManager.getTexture( wall.tex );
            float u0 = 0.0f;
            float u1 = ( wall.b - wall.a ).length() * currentTex.getWidth();

            // BAREBONES Z-CLIPPING
            // 1. If BOTH points are behind the camera, skip completely to prevent NaN division!
            if ( p0.y < 0.001f && p1.y < 0.001f ) continue;

            // Save original z-depths
            float p0_y_Origin = p0.y;
            float p1_y_Origin = p1.y;

            // 2. Slide the vertices forward if they cross the camera plane
            if ( p0.y < 0.001f ) {
                float t = ( 0.001f - p0.y ) / ( p1_y_Origin - p0.y );
                p0.x = p0.x + t * ( p1.x - p0.x );
                p0.y = 0.001f;
                u0 = u0 + t * ( u1 - u0 );  // Clip texture coords
            }
            if ( p1.y < 0.001f ) {
                float t = ( 0.001f - p1_y_Origin ) / ( p0_y_Origin - p1_y_Origin );
                p1.x = p1.x + t * ( p0.x - p1.x );
                p1.y = 0.001f;
                u1 = u1 + t * ( u0 - u1 );  // Clip texture coords
            }

            // 3. PROJECT TO SCREEN
            float fovScale = Config::SCREEN_WIDTH / 2.0f;
            int tx0 = ( Config::SCREEN_WIDTH / 2 ) + static_cast<int>( ( p0.x / p0.y ) * fovScale );
            int tx1 = ( Config::SCREEN_WIDTH / 2 ) + static_cast<int>( ( p1.x / p1.y ) * fovScale );

            // 4. THE CRITICAL FIX: STRICT BACKFACE CULLING
            // If tx0 >= tx1, we are looking at the BACK of the wall. 
            // DO NOT SWAP THEM! Swapping them turns the wall inside out. We discard it.
            if ( tx0 >= tx1 ) continue; 

            // Clamp to the portal window
            int x0 = std::max( tx0, entry.x0 );
            int x1 = std::min( tx1, entry.x1 );

            // Determine if portal
            bool isPortal = ( wall.portal > 0 );
            int targetSector = isPortal ? ( wall.portal - 1 ) : -1;

            // 4. DRAW
            float sy0 = ( 0.5f * Config::SCREEN_HEIGHT ) / p0.y;
            float sy1 = ( 0.5f * Config::SCREEN_HEIGHT ) / p1.y;
            int halfH = Config::SCREEN_HEIGHT / 2;

            float zfloor = sector.zfloor;
            float zceil  = sector.zceil;
            float nzfloor = isPortal ? map.getSectors()[ targetSector ].zfloor : 0;
            float nzceil  = isPortal ? map.getSectors()[ targetSector ].zceil : 0;

            int yf0 = halfH + static_cast<int>( ( player.posZ - zfloor ) * sy0 );
            int yc0 = halfH - static_cast<int>( ( zceil - player.posZ ) * sy0 );
            int yf1 = halfH + static_cast<int>( ( player.posZ - zfloor ) * sy1 );
            int yc1 = halfH - static_cast<int>( ( zceil - player.posZ ) * sy1 );

            int nyf0 = halfH + static_cast<int>( ( player.posZ - nzfloor ) * sy0 );
            int nyc0 = halfH - static_cast<int>( ( nzceil - player.posZ ) * sy0 );
            int nyf1 = halfH + static_cast<int>( ( player.posZ - nzfloor ) * sy1 );
            int nyc1 = halfH - static_cast<int>( ( nzceil - player.posZ ) * sy1 );

            int txd = tx1 - tx0;
            if ( txd == 0 ) txd = 1;

            for ( int x = x0; x <= x1; x++ ) {
                float xp = static_cast<float>( x - tx0 ) / txd;

                // 1. Calculate camera-space ray for this exact screen column
                float halfW = Config::SCREEN_WIDTH / 2.0f;
                float rayCamX = ( x - halfW ) / halfW; 
                float rayCamY = 1.0f; // 1.0 represents "forward" in depth

                // 2. Rotate the ray to World Space based on the player's rotation
                // (Assuming you have player.angle or similar in your Player struct)
                float cosA = std::cos( player.angle );
                float sinA = std::sin( player.angle );

                // Standard 2D rotation matrix
                float rayWorldX = rayCamX * cosA + rayCamY * sinA;
                float rayWorldY = -rayCamX * sinA + rayCamY * cosA;

                float iz0 = 1.0f / p0.y;
                float iz1 = 1.0f / p1.y;
                float izx = ( 1.0f - xp ) * iz0 + xp * iz1;

                // For fog calculation
                float trueDepth = 1.0f / izx;
                
                float ux = ( ( 1.0f - xp ) * ( u0 * iz0 ) + xp * ( u1 * iz1 ) ) / izx;
                int texX = ( int )ux % currentTex.getWidth();
                if ( texX < 0 ) texX += currentTex.getWidth();

                int trueYf = yf0 + static_cast<int>( xp * ( yf1 - yf0 ) );
                int trueYc = yc0 + static_cast<int>( xp * ( yc1 - yc0 ) );

                int yf = std::clamp( trueYf, y_lo[ x ], y_hi[ x ] );
                int yc = std::clamp( trueYc, y_lo[ x ], y_hi[ x ] );

                if ( sector.tceil != -1 ) {
                    const Texture& ceilTex = textureManager.getTexture( sector.tceil );
                    float zDiff = sector.zceil - player.posZ;

                    float depthNumerator = zDiff * halfH;

                    for ( int y = y_lo[ x ]; y <= yc; y++ ) {
                        // Distance from the horizon line (center of screen)
                        float row = halfH - y; 
                        if ( row <= 0.0f ) continue; // Prevent division by zero at the horizon

                        // Inverse projection: depth = (height * fov) / screen_y
                        float depth = ( depthNumerator ) / row;

                        // Calculate absolute map coordinates
                        float mapX = player.posX + rayWorldX * depth;
                        float mapY = player.posY + rayWorldY * depth;

                        // Convert to tiling texture coordinates
                        int texX = (int)(mapX * ceilTex.getWidth()) % ceilTex.getWidth();
                        int texY = (int)(mapY * ceilTex.getHeight()) % ceilTex.getHeight();
                        if ( texX < 0 ) texX += ceilTex.getWidth();
                        if ( texY < 0 ) texY += ceilTex.getHeight();

                        // Apply lighting level and draw pixel
                        Uint32 rawColor = ceilTex.getPixel( texX, texY );
                        Uint32 finalColor = applyLightEffects( rawColor, sector.lightLevel, depth );
                        drawPixel( x, y, finalColor );
                    }
                } else {
                    drawVerLine( x, y_lo[ x ], yc, 0xFF323232 ); // Fallback solid color
                }
                if ( sector.tfloor != -1 ) {
                    const Texture& floorTex = textureManager.getTexture( sector.tfloor );
                    float zDiff = player.posZ - sector.zfloor;

                    float depthNumerator = zDiff * halfH;

                    for ( int y = yf; y <= y_hi[ x ]; y++ ) {
                        // Distance from horizon (inverted for floors)
                        float row = y - halfH;
                        if ( row <= 0.0f ) continue; 

                        float depth = ( depthNumerator ) / row;

                        float mapX = player.posX + rayWorldX * depth;
                        float mapY = player.posY + rayWorldY * depth;

                        int texX = (int)(mapX * floorTex.getWidth()) % floorTex.getWidth();
                        int texY = (int)(mapY * floorTex.getHeight()) % floorTex.getHeight();
                        if ( texX < 0 ) texX += floorTex.getWidth();
                        if ( texY < 0 ) texY += floorTex.getHeight();

                        // Apply light level and draw pixel
                        Uint32 rawColor = floorTex.getPixel( texX, texY );
                        Uint32 finalColor = applyLightEffects( rawColor, sector.lightLevel, depth );
                        drawPixel( x, y, finalColor );
                    }
                } else {
                    drawVerLine( x, yf, y_hi[ x ], 0xFF646464 ); // Fallback solid color
                }

                if ( isPortal ) {
                    // 1. Calculate TRUE mathematical coordinates for the next sector
                    int trueNyf = nyf0 + static_cast<int>( xp * ( nyf1 - nyf0 ) );
                    int trueNyc = nyc0 + static_cast<int>( xp * ( nyc1 - nyc0 ) );

                    // 2. Clamp for drawing
                    int nyf = std::clamp( trueNyf, y_lo[ x ], y_hi[ x ] );
                    int nyc = std::clamp( trueNyc, y_lo[ x ], y_hi[ x ] );

                    // --- DRAW TOP STEP (UPPER WALL) ---
                    if ( nyc > yc ) {
                        // The world height of the upper step is the difference between ceilings
                        float upperWorldHeight = zceil - nzceil;
                        float upperScreenHeight = static_cast<float>( trueNyc - trueYc );

                        if ( upperScreenHeight > 0.0f ) {
                            float vStep = ( upperWorldHeight * currentTex.getHeight() ) / upperScreenHeight;
                            int drawStart = std::max( 0, yc );
                            int drawEnd = std::min( Config::SCREEN_HEIGHT - 1, nyc );
                            
                            // Start V pos from the un-clamped ceiling
                            float vPos = ( drawStart - trueYc ) * vStep;

                            for ( int y = drawStart; y <= drawEnd; y++ ) {
                                int texY = (int)vPos % currentTex.getHeight();
                                if ( texY < 0 ) texY += currentTex.getHeight();
                                vPos += vStep;

                                // Apply lighting to texture
                                Uint32 rawColor = currentTex.getPixel( texX, texY );
                                Uint32 finalColor = applyLightEffects( rawColor, sector.lightLevel, trueDepth );
                                drawPixel( x, y, finalColor );
                            }
                        }
                    }
                    
                    // --- DRAW BOTTOM STEP (LOWER WALL) ---
                    if ( nyf < yf ) {
                        // The world height of the lower step is the difference between floors
                        float lowerWorldHeight = nzfloor - zfloor;
                        float lowerScreenHeight = static_cast<float>( trueYf - trueNyf );

                        if ( lowerScreenHeight > 0.0f ) {
                            float vStep = ( lowerWorldHeight * currentTex.getHeight() ) / lowerScreenHeight;
                            int drawStart = std::max( 0, nyf + 1 );
                            int drawEnd = std::min( Config::SCREEN_HEIGHT - 1, yf - 1 );
                            
                            // Start V pos from the un-clamped top of the lower step
                            float vPos = ( drawStart - trueNyf ) * vStep;

                            for ( int y = drawStart; y <= drawEnd; y++ ) {
                                int texY = (int)vPos % currentTex.getHeight();
                                if ( texY < 0 ) texY += currentTex.getHeight();
                                vPos += vStep;

                                // Apply lighting to texture
                                Uint32 rawColor = currentTex.getPixel( texX, texY );
                                Uint32 finalColor = applyLightEffects( rawColor, sector.lightLevel, trueDepth );
                                drawPixel( x, y, finalColor );
                            }
                        }
                    }

                    // Update the window clamping for the next sectors in the queue
                    y_lo[ x ] = std::clamp( std::max( yc, nyc ), 0, Config::SCREEN_HEIGHT - 1 );
                    y_hi[ x ] = std::clamp( std::min( yf, nyf ), 0, Config::SCREEN_HEIGHT - 1 );
                } else {
                    float worldHeight = zceil - zfloor;
                    float screenWallHeight = static_cast<float>( trueYf - trueYc );
                    if ( screenWallHeight > 0.0f ) {
                        float vStep = ( worldHeight * currentTex.getHeight() ) / screenWallHeight;

                        // Prevent drawing off-screen
                        int drawStart = std::max( 0, yc );
                        int drawEnd = std::min( Config::SCREEN_HEIGHT - 1, yf );

                        float vPos = ( drawStart - trueYc ) * vStep;

                        for ( int y = drawStart; y <= drawEnd; y++ ) {
                            int texY = (int)vPos % currentTex.getHeight();
                            if ( texY < 0 ) texY += currentTex.getHeight();
                            vPos += vStep;

                            // Apply lighting to texture
                            Uint32 rawColor = currentTex.getPixel( texX, texY );
                            Uint32 finalColor = applyLightEffects( rawColor, sector.lightLevel, trueDepth );
                            drawPixel( x, y, finalColor );
                        }
                    }
                }
            }

            if ( isPortal ) {
                renderQueue.push_back({ targetSector, x0, x1 });
            }
        }
    }
}

bool Renderer::init( SDL_Window* gwindow )
{
    bool success = true;

    grenderer = SDL_CreateRenderer( gwindow, -1, SDL_RENDERER_ACCELERATED || SDL_RENDERER_PRESENTVSYNC );
    if ( grenderer == nullptr ) {
        std::cout << "Failed to create game renderer. Error: " << SDL_GetError() << '\n';
        success = false;
    }

    gtexture = SDL_CreateTexture( grenderer, SDL_PIXELFORMAT_ARGB8888, 
        SDL_TEXTUREACCESS_STREAMING, Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT);
    if ( gtexture == nullptr ) {
        std::cout << "Failed to create game texture. Error: " << SDL_GetError() << '\n';
        success = false;
    }

    buffer.resize( Config::SCREEN_WIDTH * Config::SCREEN_HEIGHT, 0xFF000000 );
    return success;
}

void Renderer::close()
{
    SDL_DestroyTexture( gtexture );
    SDL_DestroyRenderer( grenderer );
    gtexture = nullptr;
    grenderer = nullptr;
}

void Renderer::renderClear()
{
    // Clear the buffer
    std::fill( buffer.begin(), buffer.end(), 0xFF000000 );

    for ( int i = 0; i < Config::SCREEN_WIDTH; i++ ) {
        y_hi[ i ] = Config::SCREEN_HEIGHT - 1; // Highest allowable point is the bottom of the screen
        y_lo[ i ] = 0;                         // Lowest allowable point is the top of the screen
    }

    // Clear the hardware renderer
    SDL_SetRenderDrawColor( grenderer, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear( grenderer );
}

void Renderer::renderFlush()
{
    // 1. Send your software pixel array to the GPU texture
    SDL_UpdateTexture( gtexture, nullptr, buffer.data(), Config::SCREEN_WIDTH * sizeof( Uint32 ) );
    
    // 2. Paste the GPU texture onto the current drawing target (the backbuffer)
    SDL_RenderCopy( grenderer, gtexture, nullptr, nullptr );
    
    // DELETE SDL_RenderPresent( grenderer ); FROM HERE
}

Renderer::RayHit Renderer::checkRayIntersection( Vx rayOrigin, Vx rayDirection, Vx wallStart, Vx wallEnd )
{
    RayHit hitState = { false, 0.0f, 0.0f };

    float wallDX = wallEnd.x - wallStart.x;
    float wallDY = wallEnd.y - wallStart.y;

    float denom = rayDirection.x * wallDY - rayDirection.y * wallDX;

    constexpr float EPSILON = 1e-6f;

    if ( std::abs( denom ) < EPSILON ) return hitState;

    float dx = wallStart.x - rayOrigin.x;
    float dy = wallStart.y - rayOrigin.y;

    float t = ( dx * wallDY - dy * wallDX ) / denom;

    float u = ( dx * rayDirection.y - dy * rayDirection.x ) / denom;

    if ( t < 0.0f || u < 0.0f || u > 1.0f ) return hitState;

    hitState.hit = true;
    hitState.distance = t;
    hitState.wallPos = u;

    return hitState;
}
/*
void Renderer::renderWall( const Player& player, const Wall& wall, const Sector& sector )
{
    for ( int x = 0; x < Config::SCREEN_WIDTH; x++ ) {
        float cameraX = 2 * x / float( Config::SCREEN_WIDTH ) - 1;
        float rayDirX = player.dirX + player.planeX * cameraX;
        float rayDirY = player.dirY + player.planeY * cameraX;

        Vx rayOrigin = { static_cast<float>( player.posX ), static_cast<float>( player.posY ) };
        Vx rayDirection = { static_cast<float>( rayDirX ), static_cast<float>( rayDirY ) };

        RayHit hit;

        hit = checkRayIntersection( rayOrigin, rayDirection, wall.a, wall.b );

        if ( !hit.hit ) continue;

        Vx hitPoint = {
            rayOrigin.x + rayDirection.x * hit.distance,
            rayOrigin.y + rayDirection.y * hit.distance,
        };

        float dx = hitPoint.x - player.posX;
        float dy = hitPoint.y - player.posY;

        float perpWallDist = dx * player.dirX + dy * player.dirY;

        int lineHeight = ( int )( Config::SCREEN_HEIGHT / perpWallDist );
        int drawStart = -lineHeight / 2 + Config::SCREEN_HEIGHT / 2;
                if ( drawStart < 0 ) drawStart = 0;
                int drawEnd = lineHeight / 2 + Config::SCREEN_HEIGHT / 2;
                if ( drawEnd > Config::SCREEN_HEIGHT ) drawEnd = Config::SCREEN_HEIGHT - 1;

        for ( int y = drawStart; y < drawEnd; y++ ) {
            buffer[ y * Config::SCREEN_WIDTH + x ] = 0xFFFFFFFF;
        }

    }
}
*/
SDL_Renderer* Renderer::getRenderer()
{
    return grenderer;
}
std::vector<Uint32> Renderer::getBuffer()
{
    return buffer;
}