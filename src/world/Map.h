#pragma once


#include <vector>
#include <string>

#include "WorldObjects.h"

class Map
{
public:
    bool loadMapData( const std::string& path );
    void buildTestMap();
    void printMapVectors();

    const std::vector<Sector>& getSectors() const;
    const std::vector<Wall>& getWalls() const;

    void setSectorValue( int id, float zfloor, float zceil );
private:
    std::vector<Sector> sectors;
    std::vector<Wall> walls;
};