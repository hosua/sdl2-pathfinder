#include <algorithm>
#include <iterator>
#include <iostream>

#include "app.hh"
#include "scene_manager.hh"

App* app = App::getInstance();

SceneManager::SceneManager(){}

void SceneManager::addScene(std::unique_ptr<IScene> scene){
	// If there are no scenes yet, implicitly always render & handle inputs for the first scene added.
	if (_scenes.empty()){
		scene->setRendering(true);
		scene->setHandlingInputs(true);
	}
	_scenes.push_back(std::move(scene));
};

bool SceneManager::removeScene(size_t index){
	if (index >= _scenes.size())
		return false;
	_scenes.erase(std::next(_scenes.begin(), index));
	return true;
};

bool SceneManager::removeScene(const std::string& key){
	auto itr = std::remove_if(_scenes.begin(), _scenes.end(), 
			[&](const std::unique_ptr<IScene>& scene){
				return key == scene->getKey();
			});
	if (itr == _scenes.end())
		return false;
	_scenes.erase(itr);
	return true;
};

void SceneManager::renderScenes(){
	SDL_Point mouse_pos;
	SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
	drawClear();
	for (auto itr = _scenes.begin(); itr != _scenes.end(); ++itr){
		IScene* scene = itr->get();
		if (scene->isRendering()){
			scene->render();
			
			// If we're not rendering the scene, we can probably safely assume
			// that we also don't want to handle its input events.
			if (scene->isHandlingInputs())
				scene->handleInputs();
		}
	}
	drawPresent();
};

void SceneManager::renderScenesWithoutPresent(){
	SDL_Point mouse_pos;
	SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
	drawClear();
	for (auto itr = _scenes.begin(); itr != _scenes.end(); ++itr){
		IScene* scene = itr->get();
		if (scene->isRendering()){
			scene->render();
		}
	}
}

void SceneManager::drawClear(const SDL_Color& color) const {
	SDL_Renderer* renderer = app->getRenderer();
	if (!renderer) {
		std::cerr << "Error: renderer is null in drawClear\n";
		return;
	}
	const SDL_Color& c = color;
	SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
	int result = SDL_RenderClear(renderer);
	if (result != 0) {
		std::cerr << "SDL_RenderClear Error: " << SDL_GetError() << '\n';
	}
}

void SceneManager::drawPresent() const {
	SDL_Renderer* renderer = app->getRenderer();
	if (!renderer) {
		std::cerr << "Error: renderer is null in drawPresent\n";
		return;
	}
	SDL_RenderPresent(renderer);
}

// disables all scenes (input and rendering) except for the scene being switched to
bool SceneManager::switchScene(const std::string& key, bool restart){
	auto itr = std::find_if(_scenes.begin(), _scenes.end(), 
				[&](std::unique_ptr<IScene>& scene){
					return scene->getKey() == key;
				});
	if (itr == _scenes.end()){
		std::cerr << "Could not switch to scene: " << key << " because it was not found\n";
		return false;
	}

	for (std::unique_ptr<IScene>& scene : _scenes){
		scene->setRendering(false);
		scene->setHandlingInputs(false);
		if (scene->getKey() == key){
			scene->setRendering(true);
			scene->setHandlingInputs(true);
			if (restart)
				scene->restart();
		}
	}
	std::cout << "Switched to scene: " << key << '\n';
	return true;
}
// enables rendering and inputs management for the scene being launched but
// does not modify the flags for the other scenes.
bool SceneManager::launchScene(const std::string& key){
	// TODO TODO TODO
}

void SceneManager::handleAllSceneInputs(){
	for (std::unique_ptr<IScene>& scene : _scenes){
		if (scene->isHandlingInputs())
			scene->handleInputs();
	}
}
