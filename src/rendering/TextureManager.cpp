#include <iostream>
#include <format>
#include <fstream>
#include <sstream>

#include "TextureManager.h"

TextureManager::TextureManager()
{
    createFallbackTexture();
}

void TextureManager::createFallbackTexture()
{
    m_fallbackTexture.setWidth( 2 );
    m_fallbackTexture.setHeight( 2 );
    m_fallbackTexture.setPixels(
        {
            0xFFFF00FF, 0xFF000000, // Magenta, black
            0xFF000000, 0xFFFF00FF  // Black, magenta
        }
    );
}

bool TextureManager::loadTextureManifest( const std::string& manifestPath )
{
    std::ifstream file( manifestPath.c_str() );
    if ( !file ) {
        std::cout << "Failed to load Texture Manifest: " << manifestPath << '\n';
        return false;
    }

    std::string line;

    while ( std::getline( file, line ) ) {
        if ( line[0] == '#' || line[ 0 ] == '[' || line.empty() ) continue;

        std::stringstream ss( line );

        int id;
        std::string path;
        
        if ( ss >> id >> path ) {
            Texture tex;
            if ( tex.loadTexture( path ) ) {
                m_textures[ id ] = tex;
            } else {
                std::cout << "Texture missing at ID " << id << ": " << path << '\n';
                m_textures[ id ] = m_fallbackTexture;
            }
        }
    }

    return true;
}

const Texture& TextureManager::getTexture( int id ) const
{
    auto it = m_textures.find( id );
    if ( it == m_textures.end() || id == -1 ) return m_fallbackTexture;
    return it->second;
}