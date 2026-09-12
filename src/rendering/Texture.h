#pragma once

#include <SDL2/SDL.h>
#include <iostream>
#include <vector>

class Texture
{
public:
    bool loadTexture( const std::string& path );
    Uint32 getPixel( int x, int y ) const;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    void setWidth( int w ) { m_width = w; }
    void setHeight( int h ) { m_height = h; }
    void setPixels( std::vector<Uint32> p ) { m_pixels = p; }

private:
    int m_width = 0;
    int m_height = 0;

    std::vector<Uint32> m_pixels;
};