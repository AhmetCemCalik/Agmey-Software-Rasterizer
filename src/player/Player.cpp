#include <iostream>
#include <cmath>

#include "Player.h"
#include "Config.h"

void Player::updatePlayer( const Map& map, const Uint8* keyState, float frameTime )
{
    // Direction vectors based on 0 degrees = +Y (North)
    float dirX = std::sin( angle );
    float dirY = std::cos( angle );

    float speed = Config::MOVE_SPEED;
    float rotSpeed = Config::ROT_SPEED;

    float timeMultiplier = frameTime * 125.0f;

    Vx currentPos = { posX, posY };

    // Move forward (W) and backward (S) along the viewing axis
    if ( keyState[ SDL_SCANCODE_W ] ) {
        Vx targetPos = { posX + dirX * speed * timeMultiplier, posY + dirY * speed * timeMultiplier };
        checkPlayerWallCol( currentPos, targetPos, map );
        posX = targetPos.x;
        posY = targetPos.y;
    }
    if ( keyState[ SDL_SCANCODE_S ] ) {
        Vx targetPos = { posX - dirX * speed * timeMultiplier, posY - dirY * speed * timeMultiplier };
        checkPlayerWallCol( currentPos, targetPos, map );
        posX = targetPos.x;
        posY = targetPos.y;
    }
    
    // Look Left (A) and Right (D)
    if ( keyState[ SDL_SCANCODE_A ] ) {
        angle -= rotSpeed * timeMultiplier; // Turn Counter-Clockwise
    }
    if ( keyState[ SDL_SCANCODE_D ] ) {
        angle += rotSpeed * timeMultiplier; // Turn Clockwise
    }

    // Jumping
    if ( keyState[ SDL_SCANCODE_SPACE ] ) {
        
    }

    // Update eye level
    float currentFloor = map.getSectors()[ sector ].zfloor;

    // Interpolate
    float targetZ = currentFloor + 0.8f;
    posZ += ( targetZ - posZ ) * 0.15f;
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

void Player::checkPlayerWallCol( Vx currentPos, Vx& targetPos, const Map& map )
{
    int fw = map.getSectors()[ sector ].firstWall;
    int wc = map.getSectors()[ sector ].wallCount;

    for ( int x = fw; x < fw + wc; x++ ) {
        Wall wall = map.getWalls()[ x ];

        if ( checkLineIntersection( currentPos, targetPos, wall.a, wall.b ) ) {
            
            bool isSolid = ( wall.portal == 0 );
            bool stepTooHigh = false;
            bool ceilingTooLow = false;

            int maxSlides = 3;

            if ( !isSolid ) {
                int neighborIndex = wall.portal - 1;
                float currentFloor = map.getSectors()[ sector ].zfloor;
                float targetFloor = map.getSectors()[ neighborIndex ].zfloor;
                float targetCeiling = map.getSectors()[ neighborIndex ].zceil;
                
                // If the step is higher than 0.5, or the ceiling gives less than 0.8 clearance
                stepTooHigh = ( targetFloor - currentFloor > 0.5f );
                ceilingTooLow = ( targetCeiling - targetFloor < 0.8f );
            }

            if ( isSolid || stepTooHigh || ceilingTooLow ) {
                // Player is colliding and sliding on the wall
                Vx M = targetPos - currentPos;
                Vx W = wall.b - wall.a;
                Vx W_hat = W.normalize();

                float dp = M.dot( W_hat );
                Vx slideMovement = W_hat * dp;
                targetPos = currentPos + slideMovement;

                if ( maxSlides > 0 ) {
                    maxSlides--;
                    x = fw - 1;
                }

            } else {
                // Player successfully crossed a valid portal
                sector = wall.portal - 1;
                return;
            }
        }
    }
}