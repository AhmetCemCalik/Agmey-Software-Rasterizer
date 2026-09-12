#include <iostream>
#include <format>
#include <fstream>
#include <sstream>

#include "Map.h"

bool Map::loadMapData( const std::string& path )
{

    std::ifstream file( path.c_str() );
    if ( !file ) {
        std::cout << "Failed to load Map Data: " << path << '\n';
        return false;
    }

    enum Section {
        None,
        Textures,
        Sectors,
        Walls
    };

    Section section = Section::None;

    std::string line;
    
    while( std::getline( file, line ) ) {

        if ( line == "[SECTORS]" ) {
            section = Section::Sectors;
            continue;
        }
        if ( line == "[WALLS]" ) {
            section = Section::Walls;
            continue;
        }
        if ( line == "[TEXTURES]") {
            section = Section::Textures;
            continue;
        }
        if ( !line.empty() && line[0] == '#' ) continue;
        if ( line.empty() ) continue;

        std::stringstream ss( line );

        if ( section == Section::Sectors ) {
            Sector sector;
            
            ss >> sector.id
                >> sector.firstWall
                >> sector.wallCount
                >> sector.zfloor
                >> sector.zceil
                >> sector.tfloor
                >> sector.tceil
                >> sector.lightLevel;

            sectors.push_back( sector );
        }
        else if ( section == Section::Walls ) {
            Wall wall;
            
            ss >> wall.a.x 
                >> wall.a.y
                >> wall.b.x 
                >> wall.b.y
                >> wall.portal
                >> wall.tex;

            walls.push_back( wall );
        }
        else if ( section == Section::Textures ) {
            
        }
    }

    printMapVectors();

    return true;
}

void Map::printMapVectors()
{
    std::cout << "Sector data acquired from loaded text file:\n";
    for ( Sector sector: sectors ) 
        std::cout << std::format( "ID: {}, fw: {}, wc: {}, zfloor: {}, zceil:{}\n", 
            sector.id, sector.firstWall, sector.wallCount, sector.zfloor, sector.zceil );
            
    std::cout << "\nWall data acquired from the loaded text file:\n";
    for ( Wall wall: walls ) {
        std::cout << std::format( "Ax: {}, Ay y: {}, Bx: {}, By: {}, Portal: {}\n", 
            wall.a.x, wall.a.y, wall.b.x, wall.b.y, wall.portal );
    }
}


void Map::buildTestMap() {
    sectors.clear();
    walls.clear();

    // Sector 0 (Main Room) - ID 1, Vector Index 0
    sectors.push_back({ 1, 0, 6, 0.0f, 2.0f });
    walls.push_back({ {-2.0f, -2.0f}, {-2.0f,  2.0f}, 0 }); // Left (Solid)
    walls.push_back({ {-2.0f,  2.0f}, {-1.0f,  2.0f}, 0 }); // Front Left (Solid)
    walls.push_back({ {-1.0f,  2.0f}, { 1.0f,  2.0f}, 2 }); // PORTAL to Sector ID 2
    walls.push_back({ { 1.0f,  2.0f}, { 2.0f,  2.0f}, 0 }); // Front Right (Solid)
    walls.push_back({ { 2.0f,  2.0f}, { 2.0f, -2.0f}, 0 }); // Right (Solid)
    walls.push_back({ { 2.0f, -2.0f}, {-2.0f, -2.0f}, 0 }); // Back (Solid)

    // Sector 1 (Hallway) - ID 2, Vector Index 1
    sectors.push_back({ 2, 6, 4, 0.5f, 1.5f });
    walls.push_back({ {-1.0f,  2.0f}, {-1.0f,  5.0f}, 0 }); // Left (Solid)
    walls.push_back({ {-1.0f,  5.0f}, { 1.0f,  5.0f}, 0 }); // Back (Solid)
    walls.push_back({ { 1.0f,  5.0f}, { 1.0f,  2.0f}, 0 }); // Right (Solid)
    walls.push_back({ { 1.0f,  2.0f}, {-1.0f,  2.0f}, 1 }); // PORTAL to Sector ID 1
}

const std::vector<Sector>& Map::getSectors() const
{
    return sectors;
}

const std::vector<Wall>& Map::getWalls() const
{
    return walls;
}

void Map::setSectorValue( int id, float zfloor, float zceil )
{
    sectors[ id ].zfloor = zfloor;
    sectors[ id ].zfloor = zceil;
}