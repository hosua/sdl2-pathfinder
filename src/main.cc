#include <SDL2/SDL.h>

#include <chrono>
#include <iostream>
#include <random>
#include <memory>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "app.hh"
#include "scenes/game.hh"
#include "scenes/main_menu.hh"
#include "scenes/settings.hh"

Settings g_settings;

#ifdef __EMSCRIPTEN__
void emscripten_main_loop() {
	App::getInstance()->mainLoopIteration();
}
#endif

int main(){
	srand(time(NULL));

	if (!App::getInstance()->isRunning())
		return EXIT_FAILURE;

	std::unique_ptr<Scenes::MainMenu> main_menu_scene = 
		std::make_unique<Scenes::MainMenu>();

	App::getInstance()->addScene(std::move(main_menu_scene));
	App::getInstance()->addScene(std::make_unique<Scenes::Game>());
	App::getInstance()->addScene(std::make_unique<Scenes::Settings>());
	
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(emscripten_main_loop, 0, 1);
#else
	App::getInstance()->mainLoop();
#endif
	
	std::cout << "Process exited with no errors.\n";
	return EXIT_SUCCESS;
}



