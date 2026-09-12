#pragma once

#include <cmath>

struct Vx {
    float x;
    float y;

    Vx operator+( const Vx& v ) const { return { x + v.x, y + v.y }; }
    Vx operator-( const Vx& v ) const { return { x - v.x, y - v.y }; }
    Vx operator*( float s ) const { return { x * s, y * s }; }

    float dot( const Vx& v ) {
        return ( x * v.x ) + ( y * v.y );
    }

    float length() {
        return std::sqrt( x * x + y * y );
    }

    Vx normalize() {
        float len = length();
        if ( len == 0 ) return { 0.0f, 0.0f };
        return { x / len, y / len };
    }
};

struct Wall {
    Vx a, b;

    int portal = 0;

    int tex = -1;
};

struct Sector {
    int id;

    int firstWall = 0;
    int wallCount = 0;

    float zfloor;
    float zceil;

    int tfloor = -1;
    int tceil = -1;

    float lightLevel = 1.0f;
};

struct Object {
    
};