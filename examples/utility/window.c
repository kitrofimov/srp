#include <srp/color.h>
#include "window.h"

Window* newWindow(size_t width, size_t height, char* title, bool fullscreen)
{
	Window* this = malloc(sizeof(Window));

	this->window = SDL_CreateWindow(
		title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		width, height, fullscreen ? SDL_WINDOW_FULLSCREEN : 0
	);
	this->renderer = SDL_CreateRenderer(this->window, -1, 0);
	this->texture = SDL_CreateTexture(
		this->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
		width, height
	);

	this->running = true;
	
	return this;
}

void freeWindow(Window* this)
{
	SDL_DestroyWindow(this->window);
	SDL_DestroyRenderer(this->renderer);
	SDL_DestroyTexture(this->texture);
	free(this);
}

void windowPollEvents(Window* this)
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_QUIT:
				this->running = false;
				break;
			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_ESCAPE)
					this->running = false;
				break;
		}
	}
}

void windowPresent(const Window* this, const SRPFramebuffer* fb)
{
	SDL_UpdateTexture(this->texture, NULL, fb->color, fb->width * sizeof(SRPColor));
	SDL_RenderCopy(this->renderer, this->texture, NULL, NULL);
	SDL_RenderPresent(this->renderer);
}
