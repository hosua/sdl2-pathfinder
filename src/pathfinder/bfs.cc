
#include <chrono>
#include <functional>
#include <map>
#include <queue>
#include <set>
#include <utility>
#include <vector>

#include "../defs.hh"
#include "bfs.hh"

#include "app.hh"

static std::vector<SDL_Point> s_moves = {{0, +1}, {+1, 0}, {-1, 0}, {0, -1}};

std::vector<SDL_Point> PathFinder::bfs(World &world, const int &search_speed) {
  using std::vector, std::function, std::map, std::pair, std::make_pair;

  const int BLOCK_W = g_settings.block_size;
  const int BLOCK_H = g_settings.block_size;

  SDL_Renderer *renderer = App::getInstance()->getRenderer();
  vector<SDL_Point> path;
  std::set<pair<int, int>> vis;
  map<pair<int, int>, SDL_Point> parent;

  function<void(SDL_Point, World &, vector<SDL_Point>, vector<SDL_Point> &,
                std::set<pair<int, int>> &)>
      bfs_helper;

  const int search_delay = PathFinder::SEARCH_SPEED_MAP.at(search_speed);

  SDL_Point start = world.getPlayerPos();
  SDL_Point goal = world.getEndPos();

  std::queue<SDL_Point> q; // <curr_node, parent>
  q.push(start);
  parent[make_pair(start.x, start.y)] = {-1, -1};

  std::vector<SDL_Rect> search_markers;
  vis.insert(make_pair(start.x, start.y));

  const SDL_Color c = Color::GREEN;
  while (!q.empty()) {
    size_t breadth = q.size();
    while (breadth--) {
      SDL_Point pos = q.front();
      // printf("(%i,%i) -> ", pos.x, pos.y);
      q.pop();
      SDL_Rect rect = {world.getRect().x + pos.x * BLOCK_W,
                       world.getRect().y + (pos.y * BLOCK_H), BLOCK_W, BLOCK_H};

      // animate & reconstruct the path we formed when we reach the goal
      if (pos.x == goal.x && pos.y == goal.y) {
        SDL_Color c_finish = Color::Light::GREEN;
        // reconstruct
        pair<int, int> crawl = make_pair(pos.x, pos.y);
        SDL_Point node = {crawl.first, crawl.second};
        path.push_back(node);
        std::vector<SDL_Rect> path_rects;
        while (crawl.first != start.x || crawl.second != start.y) {
          node = {crawl.first, crawl.second};
          path.push_back(node);
          SDL_Point p = parent[crawl];
          crawl.first = p.x, crawl.second = p.y;
          SDL_Rect path_rect = {world.getRect().x + p.x * BLOCK_W,
                                p.y * BLOCK_H, BLOCK_W, BLOCK_H};
          path_rects.push_back(path_rect);

          App::getInstance()->renderScenesWithoutPresent();
          SDL_SetRenderDrawColor(renderer, c_finish.r, c_finish.g, c_finish.b,
                                 128);
          for (const SDL_Rect &pr : path_rects) {
            SDL_RenderFillRect(renderer, &pr);
          }
          App::getInstance()->drawPresent();

          // add some delay to the path reconstructing animation
          SDL_Delay(7);
        }

        // remove starter node in path (player is already on here)
        if (path.size() > 0)
          path.erase(path.begin());
        std::reverse(path.begin(), path.end());
        return path;
      }

      search_markers.push_back(rect);

      App::getInstance()->renderScenesWithoutPresent();
      SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 128);
      for (const SDL_Rect &marker : search_markers) {
        SDL_RenderFillRect(renderer, &marker);
      }
      App::getInstance()->drawPresent();
      App::getInstance()->delayHighRes(search_delay);

      for (const SDL_Point &moves : s_moves) {
        SDL_Point n = {pos.x + moves.x, pos.y + moves.y};
        pair<int, int> pr = make_pair(n.x, n.y);
        if (world.inBounds(n.x, n.y) &&
            (world.getEntityAt(n.x, n.y) == ENT_NONE ||
             world.getEntityAt(n.x, n.y) == ENT_END) &&
            vis.find(pr) == vis.end()) {
          q.push(n);
          parent[make_pair(n.x, n.y)] = pos;
          vis.insert(
              pr); // Mark as visited when adding to queue, not when popping
        }
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

  return {}; // no path found
}
