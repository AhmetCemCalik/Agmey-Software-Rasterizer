#include <iostream>

#include "Game.h"
#include "Config.h"

bool Game::init()
{
    bool success = true;

    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        std::cout << "Failed to initialize sdl. Error: " << SDL_GetError() << '\n';
        success = false;
    } else {
        gwindow = SDL_CreateWindow( "Agmey's FPS", SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED, Config::SCREEN_WIDTH, Config::SCREEN_HEIGHT, 0 );
        if ( gwindow == NULL ) {
            std::cout << "Failed to create window. Error: " << SDL_GetError() << '\n';
            success = false;
        }

        int imgFlags = IMG_INIT_PNG;
        if ( !( IMG_Init( imgFlags ) & imgFlags ) ) {
            std::cout << "Failed to initialize image subsystem. Error: " << IMG_GetError() << '\n';
            success = false;
        }

        if ( TTF_Init() < 0 ) {
                std::cout << "Failed to initialize TTF. Error: " << TTF_GetError() << '\n';
                success = false;
            }
        textRenderer.font = TTF_OpenFont("/System/Library/Fonts/Geneva.ttf", 12);
    }

    return success;
}

void Game::close()
{
    SDL_DestroyWindow( gwindow );
    gwindow = NULL;

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Game::run()
{
    if ( !init() ) {
        std::cout << "Failed to initialize the game.\n";
    } else {

        map.loadMapData( "assets/levels/level-complex.txt" );
        textureManager.loadTextureManifest( "assets/textures/textures.txt" );

        player.sector = 0;

        double time = 0.0;
        double oldTime = 0.0;

        // float lastTime = SDL_GetTicks();

        if ( !renderer.init( gwindow ) ) {
            std::cout << "Failed to initialize the renderer class.\n";
        } else {

            SDL_Event e;
            bool quit = false;
            while( !quit ) {

                while ( SDL_PollEvent( &e ) != 0 ) {
                    if ( e.type == SDL_QUIT ) {
                        quit = true;
                    }
                }

                // 1. Time & FPS Calculation with Divide-By-Zero protection
                oldTime = time;
                time = SDL_GetTicks();
                float frameTime = ( time - oldTime ) / 1000.0f;
                if ( frameTime <= 0.0f ) frameTime = 0.001f;
                if ( frameTime > 0.1f ) frameTime = 0.1f;

                const Uint8* keyState = SDL_GetKeyboardState( NULL );
                player.updatePlayer( map, keyState, frameTime );

                /*
                float currentTime = SDL_GetTicks();
                float deltaTime = ( currentTime - lastTime ) / 1000.0f;
                lastTime = currentTime;
                */
                
                int currentFPS = static_cast<int>( 1.0 / frameTime );
                std::string fpsText = " FPS: " + std::to_string( currentFPS );
                
                // 2. The Render Pipeline
                renderer.renderClear();
                renderer.renderFrame( player, map, textureManager ); 
                renderer.renderFlush(); // Assumes this now ONLY does SDL_RenderCopy for your 3D buffer

                // 3. Draw UI on top of the 3D buffer
                textRenderer.renderText( renderer.getRenderer(), fpsText.c_str() );
                
                // 4. Finally, present the composited frame to the monitor
                SDL_RenderPresent( renderer.getRenderer() );

                // SDL_Delay( 4 );
            }
            
            renderer.close();
        }
    }

    close();
}