/* 
 * File:   main.c
 * Author: lujji
 *
 * Created on September 25, 2016, 9:11 PM
 */
#include <SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void clean_up();

//Loads individual image as texture
SDL_Texture* loadTexture(char *);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

bool init() {
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        success = false;
    } else {
        //Set texture filtering to linear
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            printf("Warning: Linear texture filtering not enabled!");
        }

        //Create window
        gWindow = SDL_CreateWindow("SDL Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == NULL) {
            printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
            success = false;
        } else {
            //Create renderer for window
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
            if (gRenderer == NULL) {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
                success = false;
            } else {
                //Initialize renderer color
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
            }
        }
    }

    return success;
}

bool loadMedia() {
    //Loading success flag
    bool success = true;

    //Nothing to load
    return success;
}

void clean_up() {
    //Destroy window	
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}

SDL_Texture* loadTexture(char *str) {
    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = SDL_LoadBMP(str);
    if (loadedSurface == NULL) {
        printf("Unable to load image! SDL_image Error\n");
    } else {
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL) {
            printf("Unable to create texture from! SDL Error: %s\n", SDL_GetError());
        }

        //Get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }

    return newTexture;
}

int main(int argc, char* args[]) {
    if (!init()) {
        printf("Failed to initialize!\n");
    } else {
        if (!loadMedia()) {
            printf("Failed to load media!\n");
        } else {
            bool quit = false;
            SDL_Event e; //Event handler

            int x_ofs = 0, y_ofs = 0;

            while (!quit) {
                while (SDL_PollEvent(&e) != 0) {
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    } else if (e.type == SDL_KEYDOWN) {
                        switch (e.key.keysym.sym) {
                            case SDLK_UP:
                                y_ofs--;
                                break;
                            case SDLK_DOWN:
                                y_ofs++;
                                break;
                            case SDLK_LEFT:
                                x_ofs--;
                                break;
                            case SDLK_RIGHT:
                                x_ofs++;
                                break;
                            case SDLK_ESCAPE:
                                quit = true;
                                break;
                            default:
                                //gTexture = SDL_LoadBMP("image.bmp");
                                break;
                        }
                    }
                }

                //Clear screen
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0x00);
                SDL_RenderClear(gRenderer);

                //Render red filled quad
                SDL_Rect fillRect = {SCREEN_WIDTH / 4 + x_ofs * 4, SCREEN_HEIGHT / 4 + y_ofs * 4, SCREEN_WIDTH / 32, SCREEN_WIDTH / 32};
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
                SDL_RenderFillRect(gRenderer, &fillRect);

                //Render green outlined quad
                SDL_Rect outlineRect = {SCREEN_WIDTH / 6, SCREEN_HEIGHT / 6, SCREEN_WIDTH * 2 / 3, SCREEN_HEIGHT * 2 / 3};
                SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);
                SDL_RenderDrawRect(gRenderer, &outlineRect);

                //Draw blue horizontal line
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0x00, 0xFF);
                SDL_RenderDrawLine(gRenderer, 0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2);

                //Draw vertical line of yellow dots
                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0x00, 0xFF);
                for (int i = 0; i <= SCREEN_HEIGHT; i += 8) {
                    SDL_RenderDrawPoint(gRenderer, SCREEN_WIDTH / 2, i);
                }

                //Update screen
                SDL_RenderPresent(gRenderer);
            }
        }
    }

    clean_up();

    return 0;
}
