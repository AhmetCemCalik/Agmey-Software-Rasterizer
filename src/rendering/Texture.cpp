#include <SDL2/SDL_image.h>

#include "Texture.h"
#include "Config.h"


bool Texture::loadTexture( const std::string& path )
{
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if ( loadedSurface == nullptr ) {
        std::cout << "Failed to load the image to surface. Error: " << SDL_GetError() << '\n';
        return false;
    }

    SDL_Surface* formattedSurface = SDL_ConvertSurfaceFormat( loadedSurface, SDL_PIXELFORMAT_ARGB8888, 0 );
    SDL_FreeSurface( loadedSurface ); loadedSurface = nullptr;
    if ( formattedSurface == nullptr ) {
        std::cout << "Failed to format the surface. Error: " << SDL_GetError() << '\n';
        return false;
    }

    m_width = formattedSurface->w; m_height = formattedSurface->h;
    m_pixels.resize( m_height * m_width );

    SDL_LockSurface( formattedSurface );
    Uint32* src = static_cast<Uint32*>( formattedSurface->pixels );
    for ( int y = 0; y < m_height; y++ )
    for ( int x = 0; x < m_width; x++ ) {
        m_pixels[ y * m_width + x ] = src[ y * formattedSurface->pitch / sizeof( Uint32 ) + x ];
    }
    SDL_UnlockSurface( formattedSurface );

    SDL_FreeSurface( formattedSurface );
    formattedSurface = nullptr;

    return true;
}

Uint32 Texture::getPixel( int x, int y ) const
{
    if ( x < 0 ) x = 0;
    if ( y < 0 ) y = 0;
    if ( x > m_width ) x = m_width - 1;
    if ( y > m_height ) y = m_height - 1;

    return m_pixels[ y * m_width + x ];
}