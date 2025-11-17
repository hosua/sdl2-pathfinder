#include <functional>
#include <iostream>
#include <set>
#include <stack>
#include <utility>
#include <vector>

#include "dfs.hh"

#include "../defs.hh"
#include "app.hh"
#include "world.hh"

using namespace PathFinder;

static std::vector<SDL_Point> s_moves = {{0, +1}, {+1, 0}, {-1, 0}, {0, -1}};

std::vector<SDL_Point> PathFinder::dfs(World &world, const int &search_speed) {
  const int BLOCK_W = g_settings.block_size;
  const int BLOCK_H = g_settings.block_size;

  SDL_Renderer *renderer = App::getInstance()->getRenderer();
  std::vector<SDL_Point> path;

  const int search_delay = SEARCH_SPEED_MAP.at(search_speed);

  SDL_Point goal = world.getEndPos();
  SDL_Point start = world.getPlayerPos();

  // set render color for path search marking
  SDL_Color c = Color::GREEN;
  std::vector<SDL_Rect> search_markers;

  // Use iterative DFS with a stack to avoid stack overflow
  struct DFSNode {
    SDL_Point pos;
    std::vector<SDL_Point> path;
  };

  std::stack<DFSNode> st;
  std::set<std::pair<int, int>> vis;

  DFSNode start_node;
  start_node.pos = start;
  start_node.path.push_back(start);
  st.push(start_node);
  vis.insert(std::make_pair(start.x, start.y));

  while (!st.empty()) {
    DFSNode node = st.top();
    st.pop();

    SDL_Point pos = node.pos;
    std::vector<SDL_Point> curr_path = node.path;

    if (pos.x == goal.x && pos.y == goal.y) {
      path = curr_path;
      if (path.size() > 0)
        path.erase(path.begin());
      break;
    }

    SDL_Rect rect = {world.getRect().x + (pos.x * BLOCK_W),
                     world.getRect().y + (pos.y * BLOCK_H), BLOCK_W, BLOCK_H};
    search_markers.push_back(rect);

    App::getInstance()->renderScenesWithoutPresent();
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 128);
    for (const SDL_Rect &marker : search_markers) {
      SDL_RenderFillRect(renderer, &marker);
    }
    App::getInstance()->drawPresent();
    App::getInstance()->delayHighRes(search_delay);

    for (const SDL_Point &moves : s_moves) {
      int nx = pos.x + moves.x, ny = pos.y + moves.y;
      std::pair<int, int> pr = std::make_pair(nx, ny);
      if (world.inBounds(nx, ny) &&
          (world.getEntityAt(nx, ny) == ENT_NONE ||
           world.getEntityAt(nx, ny) == ENT_END) &&
          vis.find(pr) == vis.end()) {
        DFSNode new_node;
        new_node.pos.x = nx;
        new_node.pos.y = ny;
        new_node.path = curr_path;
        SDL_Point new_point;
        new_point.x = nx;
        new_point.y = ny;
        new_node.path.push_back(new_point);
        st.push(new_node);
        vis.insert(pr);
      }
    }
  }

  // Final render to show all visited nodes
  App::getInstance()->renderScenesWithoutPresent();
  SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 128);
  for (const SDL_Rect &marker : search_markers) {
    SDL_RenderFillRect(renderer, &marker);
  }
  App::getInstance()->drawPresent();

  // animate the path reconstruction formed
  SDL_Color c_finish = Color::Light::GREEN;
  std::vector<SDL_Rect> path_rects;
  for (auto itr = path.rbegin(); itr != path.rend(); ++itr) {
    const SDL_Point pt = *itr;
    const SDL_Rect rect = {world.getRect().x + pt.x * BLOCK_W,
                           world.getRect().y + (pt.y * BLOCK_H), BLOCK_W,
                           BLOCK_H};
    path_rects.push_back(rect);

    App::getInstance()->renderScenesWithoutPresent();
    SDL_SetRenderDrawColor(renderer, c_finish.r, c_finish.g, c_finish.b, 128);
    for (const SDL_Rect &path_rect : path_rects) {
      SDL_RenderFillRect(renderer, &path_rect);
    }
    App::getInstance()->drawPresent();
    SDL_Delay(7);
  }

  return path;
}
