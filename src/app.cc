#include <algorithm>
#include <chrono>
#include <memory>
#include <iostream>
#include <random>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "app.hh"
#include "defs.hh"
// Gets a global vector of SDL_Events, which are polled in App::mainLoop().
// This is necessary to handle events outside of while(SDL_PollEvents()).
std::vector<SDL_Event>& App::getFrameEvents(){
	static std::vector<SDL_Event> event_list;
	return event_list;
}

const SDL_Point& App::getMousePos(){
	static SDL_Point mouse_pos;
	SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
	return mouse_pos;
}

// Stops the current thread from running for float (ms) time. Converted to
// ns for higher resolution.
void App::delayHighRes(float ms){
	int ns = (ms * (float)1e6);
	auto start_time = std::chrono::steady_clock::now();
	while ((std::chrono::steady_clock::now() - start_time) < std::chrono::nanoseconds(ns))
		continue;
}


App* App::_instance = nullptr;

App::App():
	SceneManager(),
	_gen(std::random_device{}()){
	_running = initSDL() && Font::init();
}

App::~App(){
	SDL_DestroyRenderer(_renderer);
	_renderer = nullptr;
	SDL_FreeSurface(_surface);
	_surface = nullptr;
	SDL_DestroyWindow(_window);
	_window = nullptr;
	Font::close();
	SDL_Quit();
}

bool App::initSDL(){
	if (SDL_Init(SDL_INIT_VIDEO) > 0){
		std::cerr << "SDL_Init Error: " << SDL_GetError() << '\n';
		return false;
	} 

#ifdef __EMSCRIPTEN__
	_window = SDL_CreateWindow("Game Engine",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			WINDOW_W, WINDOW_H, 
			SDL_WINDOW_SHOWN);
#else
	_window = SDL_CreateWindow("Game Engine",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WINDOW_W, WINDOW_H, 
			0);

	SDL_SetWindowBordered(_window, SDL_FALSE); // remove window border
#endif

	if (!_window){
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << '\n';
		return false;
	}

#ifdef __EMSCRIPTEN__
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
	if (!_renderer){
		std::cerr << "SDL_CreateRenderer Error (accelerated): " << SDL_GetError() << '\n';
		_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_SOFTWARE);
		if (!_renderer){
			std::cerr << "SDL_CreateRenderer Error (software): " << SDL_GetError() << '\n';
			return false;
		}
	}
#else
	_renderer = SDL_CreateRenderer(_window, -1, 0);
	if (!_renderer){
		std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << '\n';
		return false;
	}
#endif

	SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND); // but will it blend?

#ifndef __EMSCRIPTEN__
	_surface = SDL_GetWindowSurface(_window);
	if (!_surface){
		std::cerr << "SDL_GetWindowSurface Error: " << SDL_GetError() << '\n';
		return false;
	}
#else
	_surface = nullptr; // Not needed for Emscripten
#endif

	TTF_Init();

	return true;
}

void App::mainLoop(){
	while (isRunning()){
		SceneManager::renderScenes();
		
		const SDL_Point& mouse_pos = getMousePos();
		SDL_Event event;
		while (SDL_PollEvent(&event)){
			getFrameEvents().push_back(event);
			switch(event.type){
				case SDL_KEYDOWN:
					{
						if (event.key.keysym.scancode == SDL_SCANCODE_M)
							fprintf(stdout, "Mouse position: (%i,%i)\n", mouse_pos.x, mouse_pos.y);
						if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
							std::cout << "ESC was pressed\n";
							setRunning(false);
						}
						break;
					}
				default:
					break;
			}
		}

		SceneManager::handleAllSceneInputs();

		getFrameEvents().clear();
		SDL_Delay(17);
	}
}

#ifdef __EMSCRIPTEN__
void App::mainLoopIteration(){
	if (!isRunning()){
		emscripten_cancel_main_loop();
		return;
	}
	
	SceneManager::renderScenes();
	
	const SDL_Point& mouse_pos = getMousePos();
	SDL_Event event;
	while (SDL_PollEvent(&event)){
		getFrameEvents().push_back(event);
		switch(event.type){
			case SDL_KEYDOWN:
				{
					if (event.key.keysym.scancode == SDL_SCANCODE_M)
						fprintf(stdout, "Mouse position: (%i,%i)\n", mouse_pos.x, mouse_pos.y);
					if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
						std::cout << "ESC was pressed\n";
						setRunning(false);
					}
					break;
				}
			default:
				break;
		}
	}

	SceneManager::handleAllSceneInputs();

	getFrameEvents().clear();
}
#endif

int App::getRandInt(int min, int max) const {
	std::uniform_int_distribution<> distr(min, max);
	return distr(_gen);
}

float App::getRandFloat(float min, float max) const {
	std::uniform_real_distribution<> distr(min, max);
	return distr(_gen);
}
