#pragma once

#include <vector>
#include <string>
#include <unordered_map>

#include "Texture.h"

class TextureManager
{
public:
    TextureManager();

    bool loadTextureManifest( const std::string& path );

    const Texture& getTexture( int id ) const;

private:
    std::unordered_map<int, Texture> m_textures;

    Texture m_fallbackTexture;

    void createFallbackTexture();
};