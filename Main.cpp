#include <iostream>
#include <SDL.h>
using namespace std;

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Initialization Failed: " << SDL_GetError() << "\n";
        return 1;
    }
    
    SDL_Window *window = SDL_CreateWindow(
        "CHIP-8 Test Window", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        800, 600, 
        SDL_WINDOW_SHOWN
    );
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    bool running = true;
    SDL_Event event;
    Uint32 startTime = SDL_GetTicks(); // 15-second timer tracker

    // Active frame loop
    while (running) {
        // 1. Continuously pump window events so Windows knows the app is alive
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false; // Closes when you click the 'X'
            }
        }

        // Auto-close after 15 seconds (you can remove this later)
        if (SDL_GetTicks() - startTime > 15000) {
            running = false;
        }

        // 2. Render every frame so Windows DWM sees active updates
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red color
        SDL_RenderClear(renderer); 
        SDL_RenderPresent(renderer);

        // 3. Cap frame rate to ~60 FPS (approx 16ms delay) so CPU doesn't spike
        SDL_Delay(16);
    }
    
    // Clean up resources cleanly
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
