#include <stdio.h>
#include <SDL2\SDL.h>
#include <chrono>
#include <thread>
#include <iostream>
#include "chip8.h"
#include "keypad.h"

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 320;

static SDL_Renderer *gRenderer;
static SDL_Texture *gTexture = NULL;
static SDL_Window *gWindow = NULL;

CHIP8 c8;

void displayUpdate()
{
    SDL_Surface *ARGBBuffer = NULL;
    ARGBBuffer = SDL_CreateRGBSurface(0, 64, 32, 32, 0, 0, 0, 0);

    SDL_LockSurface(ARGBBuffer);

    uint32_t *pixels = (uint32_t *)ARGBBuffer->pixels;
    for (int i = 0; i < 2048; i++)
    {
        pixels[i] = c8.gfx[i] == 0 ? 0 : 0xFFFFFFFF;
    }
    SDL_UnlockSurface(ARGBBuffer);

    gTexture = SDL_CreateTextureFromSurface(gRenderer, ARGBBuffer);
    SDL_RenderClear(gRenderer);
    SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
    SDL_RenderPresent(gRenderer);

    SDL_FreeSurface(ARGBBuffer);
}

void close()
{
    SDL_DestroyTexture(gTexture);
    gTexture = NULL;

    SDL_DestroyRenderer(gRenderer);
    gRenderer = NULL;

    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
}

int main(int argc, char **argv)
{
	// Load game
	if(!c8.loadROM(argv[1]))		
		return 1;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Could not initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    gWindow = SDL_CreateWindow( 
        "ch8r v0.1", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, 
        SDL_WINDOW_SHOWN );

    gRenderer = SDL_CreateRenderer( gWindow, -1, 0 );

    if (gWindow == NULL)
    {
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    bool quit = false;

    SDL_Event event;

    while(!quit)
    {
        std::chrono::time_point<std::chrono::steady_clock> startCycle = std::chrono::steady_clock::now();

        if( SDL_PollEvent( &event ) != 0)
        {
            if(event.type == SDL_QUIT)
                quit = true;

            else if(event.type == SDL_KEYDOWN)
            {
                for(int i = 0; i < 16; ++i){
                    if(event.key.keysym.sym == keymap[i])
                        c8.key[i] = 1;
                }
            }

            else if(event.type == SDL_KEYUP)
            {
                for(int i = 0; i < 16; ++i){
                    if(event.key.keysym.sym == keymap[i])
                        c8.key[i] = 0;
                }
            }
            
        }
        
        c8.emulateCycle();

        if(c8.drawFlag){
            displayUpdate();
            c8.drawFlag = false;

            std::chrono::time_point<std::chrono::steady_clock> endCycle = std::chrono::steady_clock::now();
            std::chrono::milliseconds delay = std::chrono::duration_cast<std::chrono::milliseconds>(endCycle - startCycle);
            std::this_thread::sleep_for(std::chrono::milliseconds(16)-delay);
        }

    }

    close();
    return 0;

}