#pragma once
#include <functional>
#include <iostream>

#include "../pathfinder/world.hh"
#include "../scene.hh"
#include "ui/all.hh"

#include "scene_manager.hh"

using namespace PathFinder;

namespace Scenes {
class Game : public IScene, public World {
public:
  Game();
  ~Game() = default;

  void render() override;
  void handleInputs() override;

  void restart() override;

  // gets and stores the path from player -> goal in _path.
  bool
  getPath(std::function<std::vector<SDL_Point>(World &world,
                                               std::vector<SDL_Point> path)>
              helper); // return false if no path is found
  void renderPath();   // renders _path (if one can be formed)
  void renderSearchStates();
  void renderRetrace();

  // sets the entity type that the player will emplace when clicking on the
  // world
  void setEntityType(EntType entity_type) { _entity_type = entity_type; }

  // renders a rect behind the button of which entity type is currently selected
  void renderSelectedEntityType();

private:
  // the points on the grid that have already been visited by the currently
  // running algorithm
  std::vector<SDL_Point> _search_states;
  // the path that is traced when the search finds the goal
  std::vector<SDL_Point> _retrace;
  // the final path after it is retraced
  std::vector<SDL_Point> _path;

  // the current type of entity to spawn/move when clicking
  EntType _entity_type = ENT_WALL;
  // the speed of the pathfinding search
  int _search_speed = 7;
};
} // namespace Scenes

namespace GameWidgets {
class DFSBtn : public UI::TextButton {
public:
  ~DFSBtn() = default;
  DFSBtn(World &world, std::vector<SDL_Point> &path, const int &search_speed)
      : TextButton("DFS", 5, 5, 130, 50), _world(world), _path(path),
        _search_speed(search_speed) {}

  void handleInputs() override;

private:
  World &_world;
  std::vector<SDL_Point> &_path;
  const int &_search_speed;
};

class BFSBtn : public UI::TextButton {
public:
  ~BFSBtn() = default;
  BFSBtn(World &world, std::vector<SDL_Point> &path, const int &search_speed)
      : TextButton("BFS", 5, 60, 130, 50), _world(world), _path(path),
        _search_speed(search_speed) {}

  void handleInputs() override;

private:
  World &_world;
  std::vector<SDL_Point> &_path;
  const int &_search_speed;
};

class AStarBtn : public UI::TextButton {
public:
  AStarBtn(World &world, std::vector<SDL_Point> &path, const int &search_speed)
      : TextButton("A*", 5, 115, 130, 50), _world(world), _path(path),
        _search_speed(search_speed) {}

  void handleInputs() override;

private:
  World &_world;
  std::vector<SDL_Point> &_path;
  const int &_search_speed;
};

/**
 * Generates a random map
 */

class RandomizeAlgoABtn : public UI::TextButton {
public:
  RandomizeAlgoABtn(World &world, std::vector<SDL_Point> &path)
      : TextButton("Randomize A", 5, 270, 130, 50, Font::openSansSmall),
        _world(world), _path(path) {}

  void handleInputs() override;

private:
  World &_world;
  std::vector<SDL_Point> &_path;
};

class RandomizeAlgoBBtn : public UI::TextButton {
public:
  RandomizeAlgoBBtn(World &world, std::vector<SDL_Point> &path)
      : TextButton("Randomize B", 5, 325, 130, 50, Font::openSansSmall),
        _world(world), _path(path) {}

  void handleInputs() override;

private:
  World &_world;
  std::vector<SDL_Point> &_path;
};

// TODO: Create add noise to current map button

class SelectEntWallBtn : public UI::IButton {
public:
  SelectEntWallBtn(EntType &ent_type)
      : IButton(5, 170, 40, 40, Color::GREY), _ent_type(ent_type) {}
  void handleInputs() override;

private:
  EntType &_ent_type;
};

class SelectEntPlayerBtn : public UI::IButton {
public:
  SelectEntPlayerBtn(EntType &ent_type)
      : IButton(50, 170, 40, 40, Color::BLUE), _ent_type(ent_type) {}
  void handleInputs() override;

private:
  EntType &_ent_type;
};

class SelectEntEndBtn : public UI::IButton {
public:
  SelectEntEndBtn(EntType &ent_type)
      : IButton(95, 170, 40, 40, Color::RED), _ent_type(ent_type) {}
  void handleInputs() override;

private:
  EntType &_ent_type;
};

class MainMenuBtn : public UI::TextButton {
public:
  MainMenuBtn()
      : TextButton("Main Menu", 5, WINDOW_H - 110, 130, 50,
                   Font::openSansSmall) {}
  void handleInputs() override;

private:
};

class ExitBtn : public UI::TextButton {
public:
  ExitBtn()
      : TextButton("Exit", 5, WINDOW_H - 55, 130, 50, Font::openSansSmall,
                   Color::RED) {}
  void handleInputs() override;

private:
};
} // namespace GameWidgets
