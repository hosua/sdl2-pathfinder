#pragma once

#include <SDL2/SDL.h>
#include <initializer_list>
#include <memory>
#include <vector>

#include "scene.hh"
#include "color.hh"

class SceneManager {
public:
	
	SceneManager();
	~SceneManager() = default;
	
	void addScene(std::unique_ptr<IScene> scene);
	bool removeScene(size_t index);
	bool removeScene(const std::string& key);
	void clearScenes(); /*!< Deletes all scenes from existence */
	
	void handleAllSceneInputs();

	void drawClear(const SDL_Color& color = Color::BLACK) const; // always call at the beginning of game loop iteration
	void drawPresent() const; // always call at the end of game loop iteration
	
	// disables all scenes (input and rendering) except for the scene being switched to.
	// If restart == true, will call the Scene's restart() method after switching.
	bool switchScene(const std::string& key, bool restart = false);
	
	// TODO: This will remain unimplemented until I have a use-case for overlaying scenes
	bool launchScene(const std::string& key);

	/** \brief Renders all scenes that have been added with App::addScene().
	 *
	 */
	void renderScenes();
	
	/** \brief Renders all scenes without presenting (useful for pathfinding animations).
	 *
	 */
	void renderScenesWithoutPresent();

private:
	std::vector<std::unique_ptr<IScene>> _scenes;
};

