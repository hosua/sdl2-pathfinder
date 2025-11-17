#pragma once
#include <SDL2/SDL.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <random>
#include <vector>

#include "scene.hh"
#include "scene_manager.hh"

class App : public SceneManager {
	public:
		// Singleton. This will ensure that App can only be instantiated once
		static App* getInstance(){ 
			if (_instance == nullptr)
				_instance = new App();
			return _instance;
		}
		
		// SDL_event can only be polled once in the main thread
		std::vector<SDL_Event>& getFrameEvents();
		const SDL_Point& getMousePos();
		// Stops the current thread from running for float (ms) time. Converted to
		// ns for higher resolution.
		void delayHighRes(float ms);

		void mainLoop();
#ifdef __EMSCRIPTEN__
		void mainLoopIteration();
#endif

		void handleInputs();
		bool initSDL();

		bool isRunning() const { return _running; }

		void setRunning(bool flag){ _running = flag; }
		const bool& getRunningRef(){ return std::cref(_running); }
		
		SDL_Window* getWindow(){ return _window; }
		SDL_Surface* getSurface(){ return _surface; }
		SDL_Renderer* getRenderer(){ return _renderer; }

		int getRandInt(int min, int max) const;
		float getRandFloat(float min, float max) const;

		// move player relative to current pos, returns true if player moved
		bool movePlayer(int dx, int dy); 

		App(App* const) = delete;			 // Don't implement
		App& operator=(App const&) = delete; // Don't implement
				
	private:
		static App* _instance;
		App();
		~App();

		mutable std::mt19937 _gen;

		bool _running = false;
		SDL_Point _mouse_pos;
		// TODO: Make these shared_ptr. It would have made more sense but I'm kind of an idiot
		SDL_Window* _window = nullptr;
		SDL_Surface* _surface = nullptr;
		SDL_Renderer* _renderer = nullptr;
};
