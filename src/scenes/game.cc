#include <map>
#include <set>
#include <queue>
#include <utility>

#include "game.hh"
#include "pathfinder/dfs.hh"
#include "pathfinder/bfs.hh"
#include "pathfinder/astar.hh"
#include "../defs.hh"

#include "app.hh"
#include "pathfinder/world.hh"

static App* app = App::getInstance();
static SDL_Renderer* renderer = app->getRenderer();

static const int WORLD_X = 140,
		PLAYER_MOVE_DELAY = 3,
		WORLD_Y = 0,
		WORLD_W = WINDOW_W - WORLD_X, 
		WORLD_H = WINDOW_H;

namespace Scenes {

	Game::Game():
		IScene("GAME"), 
		World(WORLD_X, WORLD_Y, WORLD_W, WORLD_H){

			std::unique_ptr<GameWidgets::DFSBtn> btn_dfs = 
				std::make_unique<GameWidgets::DFSBtn>(*this, _path, _search_speed);
			addWidget(std::move(btn_dfs));

			std::unique_ptr<GameWidgets::BFSBtn> btn_bfs = 
				std::make_unique<GameWidgets::BFSBtn>(*this, _path, _search_speed);
			addWidget(std::move(btn_bfs));

			std::unique_ptr<GameWidgets::AStarBtn> btn_astar = 
				std::make_unique<GameWidgets::AStarBtn>(*this, _path, _search_speed);
			addWidget(std::move(btn_astar));

			std::unique_ptr<GameWidgets::SelectEntPlayerBtn> btn_player = 
				std::make_unique<GameWidgets::SelectEntPlayerBtn>(_entity_type);
			addWidget(std::move(btn_player));

			std::unique_ptr<GameWidgets::SelectEntWallBtn> btn_wall = 
				std::make_unique<GameWidgets::SelectEntWallBtn>(_entity_type);
			addWidget(std::move(btn_wall));

			std::unique_ptr<GameWidgets::SelectEntEndBtn> btn_end = 
				std::make_unique<GameWidgets::SelectEntEndBtn>(_entity_type);
			addWidget(std::move(btn_end));

			std::unique_ptr<GameWidgets::RandomizeAlgoABtn> btn_rand_a =
				std::make_unique<GameWidgets::RandomizeAlgoABtn>(*this, _path);
			addWidget(std::move(btn_rand_a));

			std::unique_ptr<GameWidgets::RandomizeAlgoBBtn> btn_rand_b =
				std::make_unique<GameWidgets::RandomizeAlgoBBtn>(*this, _path);
			addWidget(std::move(btn_rand_b));

			std::unique_ptr<GameWidgets::MainMenuBtn> btn_main_menu =
				std::make_unique<GameWidgets::MainMenuBtn>();
			addWidget(std::move(btn_main_menu));

			std::unique_ptr<GameWidgets::ExitBtn> btn_exit = 
				std::make_unique<GameWidgets::ExitBtn>();
			addWidget(std::move(btn_exit));

			// search speed label
			std::unique_ptr<UI::Text> search_speed_lbl =
				std::make_unique<UI::Text>("Speed",
						10, 215,
						100,
						Font::openSansSmall);

			addWidget(std::move(search_speed_lbl));

			// search speed spinner
			std::unique_ptr<UI::Spinner<int>> search_speed_spnr =
				std::make_unique<UI::Spinner<int>>(_search_speed,
						5, 240,
						65, 25,
						0, 10, 1,
						UI::ST_HORIZONTAL);
			addWidget(std::move(search_speed_spnr));


		};

	void Game::render() {
		World::render();

		if (_path.size() > 0)
			renderPath();

		renderSelectedEntityType(); // render selected rect around entity button
		renderWidgets();
	};

	void Game::renderSelectedEntityType(){
		SDL_Rect r;
		switch (_entity_type){
			case ENT_WALL: // 1st
				r = { 4, 169, 42, 42 };
				break;
			case ENT_PLAYER: // second
				r = { 49, 169, 42, 42 };
				break;
			case ENT_END: // third
				r = { 94, 169, 42, 42 };
				break;
			case ENT_NONE:
				r = { 0, 0, 0, 0 };
				std::cerr << "Warning: ENT_NONE is selected as the entity type, but this should not be possible.\n";
				break;
		}
		SDL_Color c = Color::Light::GREEN;
		SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
		SDL_RenderDrawRect(renderer, &r);
	}

	static uint32_t s_player_last_moved = 0;
	void Game::handleInputs(){
		const int BLOCK_W = g_settings.block_size;
		const int BLOCK_H = g_settings.block_size;

		const SDL_Point& mouse_pos = app->getMousePos();
		static bool lmb_down = false, rmb_down = false;
		for (const SDL_Event& event : app->getFrameEvents()){
			switch(event.type){
				case SDL_MOUSEBUTTONDOWN:
					{
						if (event.button.button == SDL_BUTTON_LEFT)
							lmb_down = true;
						else if (event.button.button == SDL_BUTTON_RIGHT)
							rmb_down = true;
						break;
					}
				case SDL_MOUSEBUTTONUP:
					{
						if (event.button.button == SDL_BUTTON_LEFT)
							lmb_down = false;
						else if (event.button.button == SDL_BUTTON_RIGHT)
							rmb_down = false;
						break;
					}
				case SDL_KEYDOWN:
					{
						// if (event.key.keysym.scancode == SDL_SCANCODE_P)

						if (event.key.keysym.scancode == SDL_SCANCODE_1)
							setEntityType(ENT_WALL);
						if (event.key.keysym.scancode == SDL_SCANCODE_2)
							setEntityType(ENT_PLAYER);
						if (event.key.keysym.scancode == SDL_SCANCODE_3)
							setEntityType(ENT_END);
						break;
					}
				default:
					break;
			}
		}
		handleWidgetInputs();

		SDL_Point g = { (mouse_pos.x - World::getRect().x) / BLOCK_W, mouse_pos.y / BLOCK_H };
		if (lmb_down) World::spawnEntity(_entity_type, g.x, g.y);
		if (rmb_down) World::deleteWall(g.x, g.y);

		const uint8_t* kb_state = SDL_GetKeyboardState(nullptr);

		if (s_player_last_moved == 0){ // add some delay between movement events
			bool moved = false;
			if (kb_state[SDL_SCANCODE_UP] || kb_state[SDL_SCANCODE_W])
				moved |= World::movePlayerRelative(0, -1);
			if (kb_state[SDL_SCANCODE_DOWN] || kb_state[SDL_SCANCODE_S])
				moved |= World::movePlayerRelative(0, +1);
			if (kb_state[SDL_SCANCODE_LEFT] || kb_state[SDL_SCANCODE_A])
				moved |= World::movePlayerRelative(-1, 0);
			if (kb_state[SDL_SCANCODE_RIGHT] || kb_state[SDL_SCANCODE_D])
				moved |= World::movePlayerRelative(+1, 0);

			// if the player moved, reset the delay timer
			if (moved){ 
				_path.clear();
				s_player_last_moved = PLAYER_MOVE_DELAY;
			}
		} else {
			s_player_last_moved--;
		}

	}

	// gets and stores the path from player -> goal in _path.
	// helper() is a function that uses the world to find the path. It does not
	// modify world in any shape or form. helper() returns false when no path is found
	// returns false if no path is found
	bool Game::getPath(std::function<std::vector<SDL_Point>(World& world, std::vector<SDL_Point> path)> helper){ 
		_path = helper(*this, _path);
		return _path.size() > 0;
	} 

	// renders _path (if one can be formed)
	void Game::renderPath(){
		const int BLOCK_W = g_settings.block_size;
		const int BLOCK_H = g_settings.block_size;

		SDL_Color c = Color::Light::BLUE;
		// render transparent square
		SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 128);
		for (const SDL_Point pt : _path){
			SDL_Rect rect = {
				World::getRect().x + (pt.x * BLOCK_W), World::getRect().y + (pt.y * BLOCK_H), BLOCK_W, BLOCK_H
			};
			SDL_RenderFillRect(renderer, &rect);
		}
	}			

	void Game::renderSearchStates(){

	}

	void Game::renderRetrace(){

	}


	void Game::restart(){
		_path.clear();
		World::updateDimensions();
	}
}

namespace GameWidgets {
	void DFSBtn::handleInputs() {
		if (isMouseOver() && isClicked()){
			std::cout << "Finding path with DFS!\n";

			_path.clear();
			_path = PathFinder::dfs(_world, _search_speed); 

			if (_path.size() == 0){
				std::cout << "No path found!\n";
			} else {
				std::cout << "Path: \n";
				for (const SDL_Point& pt : _path)
					printf("(%i,%i) -> ", pt.x, pt.y);
				std::cout << "\n";
			}
		}
	}

	void BFSBtn::handleInputs() {
		if (isMouseOver() && isClicked()){
			std::cout << "Finding path with BFS!\n";

			_path.clear();
			_path = PathFinder::bfs(_world, _search_speed); 

			if (_path.size() == 0){
				std::cout << "No path found!\n";
			} else {
				std::cout << "Path: \n";
				for (const SDL_Point& pt : _path)
					printf("(%i,%i) -> ", pt.x, pt.y);
				std::cout << "\n";
			}
		}
	}

	void AStarBtn::handleInputs() {
		if (isMouseOver() && isClicked()){
			std::cout << "Finding path with A* search!\n";
			
			_path.clear();
			_path = PathFinder::a_star(_world, _search_speed);

			if (_path.size() == 0){
				std::cout << "No path found!\n";
			} else {
				std::cout << "Path: \n";
				for (const SDL_Point& pt : _path)
					printf("(%i,%i) -> ", pt.x, pt.y);
				std::cout << "\n";
			}

		}
	}

	void RandomizeAlgoABtn::handleInputs() {
		if (isMouseOver() && isClicked()){
			_path.clear();
			PathFinder::randomize_world_a(_world);
		}
	}

	void RandomizeAlgoBBtn::handleInputs() {
		if (isMouseOver() && isClicked()){
			_path.clear();
			PathFinder::randomize_world_b(_world);
		}
	}

	void SelectEntWallBtn::handleInputs() {
		if (isMouseOver() && isClicked())
			_ent_type = ENT_WALL;
	}

	void SelectEntPlayerBtn::handleInputs() {
		if (isMouseOver() && isClicked())
			_ent_type = ENT_PLAYER;
	}

	void SelectEntEndBtn::handleInputs() {
		if (isMouseOver() && isClicked())
			_ent_type = ENT_END;
	}

	void MainMenuBtn::handleInputs() {
		if (isMouseOver() && isClicked())
			app->switchScene("MAIN_MENU");
	}

	void ExitBtn::handleInputs() {
		if (isMouseOver() && isClicked()){
			std::cout << "Exiting the game.\n";
			app->setRunning(false);
		}
	}	
}
