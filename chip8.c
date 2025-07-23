#include <stdio.h>

#include "SDL.h"

void init_sdl(void) {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) !=0) {
		SDL_Log("Could not initialize SDL subsystems! %s\n", SDL_GetError());
		return false
	}	
	return true; // Success
}

void final_cleanup(void) {
	SDL_Quit(); // Shut down SDL subsystem
}


int main(int argc, char **argv) {
	(void) argc;
	(void) argv;

	// Initialize SDL
	if (!init_sdl()) exit(EXIT_FAILURE);

	// Final cleanup
	final_cleanip();

	exit(EXIT_SUCCESS);

}
