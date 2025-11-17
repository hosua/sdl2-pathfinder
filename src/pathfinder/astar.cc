#include "astar.hh"

#include "../defs.hh"
#include "app.hh"
#include <algorithm>
#include <iostream>
#include <memory>
#include <queue>
#include <set>

static App *app = App::getInstance();
static SDL_Renderer *renderer = app->getRenderer();

static std::vector<SDL_Point> s_moves = {{0, +1}, {+1, 0}, {-1, 0}, {0, -1}};

static int get_heuristic(int x1, int y1, int x2, int y2);

static float get_euclidean_dist(int x1, int y1, int x2, int y2) {
  return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

static int get_manhattan_dist(int x1, int y1, int x2, int y2) {
  return abs(y2 - y1) + abs(x2 - x1);
}

// g = cost of path from start to current pos (number of parent nodes)
// h = heuristic estimate of the cost to reach the goal (manhattan dist?
// **Euclidean dist**) f = g + h
struct SearchState {
  int g, h, f;
  int x, y;
  std::shared_ptr<SearchState> parent;
};

// (for debugging) print the search state info
std::ostream &operator<<(std::ostream &out, const SearchState &s) {
  out << '[' << s.f << ',' << s.g << ',' << s.h << "]: (" << s.x << ',' << s.y
      << ") -> ";
  return out;
}

// necessary for priority_queue
bool operator<(const SearchState &a, const SearchState &b) {
  return a.f > b.f; // min heap on f = g + h
}

class AStarPriorityQueue : public std::priority_queue<SearchState> {
public:
  std::shared_ptr<SearchState> find(const int x, const int y) {
    for (auto itr = c.begin(); itr != c.end(); ++itr) {
      SearchState state = *itr;
      if (x == state.x && y == state.y)
        return std::make_shared<SearchState>(*itr);
    }
    return nullptr;
  }
};

static int get_heuristic(int x1, int y1, int x2, int y2) {
  return get_manhattan_dist(x1, y1, x2, y2);
  // return floor(get_euclidean_dist(x1, y1, x2, y2));
};

std::vector<SDL_Point> PathFinder::a_star(World &world,
                                          const int &search_speed) {
  const int BLOCK_W = g_settings.block_size;
  const int BLOCK_H = g_settings.block_size;
  const int search_delay = PathFinder::SEARCH_SPEED_MAP.at(search_speed);

  std::vector<SDL_Point> path;

  int g_count = 0;

  SDL_Point start = world.getPlayerPos();
  SDL_Point goal = world.getEndPos();

  int start_h = get_heuristic(start.x, start.y, goal.x, goal.y);

  SearchState start_node = {g_count, start_h, g_count + start_h,
                            start.x, start.y, nullptr};

  g_count++;

  std::set<std::pair<int, int>> closed;
  std::vector<SDL_Rect> search_markers;

  AStarPriorityQueue pq;
  pq.push(start_node);

  const SDL_Color c = Color::GREEN;

  while (!pq.empty()) {
    SearchState node = pq.top();
    pq.pop();

    std::cout << node;
    int x = node.x, y = node.y;
    closed.insert(std::make_pair(x, y));

    SDL_Rect current_rect = {world.getRect().x + (x * BLOCK_W),
                             world.getRect().y + (y * BLOCK_H), BLOCK_W,
                             BLOCK_H};
    search_markers.push_back(current_rect);

    App::getInstance()->renderScenesWithoutPresent();
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 128);
    for (const SDL_Rect &marker : search_markers) {
      SDL_RenderFillRect(renderer, &marker);
    }
    App::getInstance()->drawPresent();
    App::getInstance()->delayHighRes(search_delay);

    if (x == goal.x && y == goal.y) {
      // std::cout << "\nReconstructing the path...\n";
      // crawl
      SDL_Color c_finish = Color::Light::GREEN;
      std::vector<SDL_Rect> path_rects;
      for (SearchState *crawl = node.parent.get(); crawl;
           crawl = crawl->parent.get()) {
        // std::cout << '(' << crawl->x << ',' << crawl->y << ")\n";
        path.push_back({crawl->x, crawl->y});
        SDL_Rect rect = {world.getRect().x + crawl->x * BLOCK_W,
                         crawl->y * BLOCK_H, BLOCK_W, BLOCK_H};
        path_rects.push_back(rect);

        App::getInstance()->renderScenesWithoutPresent();
        SDL_SetRenderDrawColor(renderer, c_finish.r, c_finish.g, c_finish.b,
                               128);
        for (const SDL_Rect &pr : path_rects) {
          SDL_RenderFillRect(renderer, &pr);
        }
        App::getInstance()->drawPresent();

        App::getInstance()->delayHighRes(search_delay);
      }
      std::reverse(path.begin(), path.end());
      return path;
    }

    // "expand" the current node
    for (const auto &[mx, my] : s_moves) {
      int nx = x + mx, ny = y + my;
      if (world.inBounds(nx, ny) &&
          world.getEntityAt(nx, ny) != ENT_WALL && // is not blocked
          closed.find(std::make_pair(nx, ny)) ==
              closed.end()) { // has not already been marked closed

        int h = get_heuristic(nx, ny, goal.x, goal.y);
        SearchState new_node = {
            g_count,
            h,
            g_count + h, // f
            nx,
            ny,
            std::make_shared<SearchState>(
                node) // parent of the new node is the current node
        };

        std::shared_ptr<SearchState> open_node;
        if ((open_node = pq.find(nx, ny))) {
          // check update parent of node based on g value
          if (g_count < open_node->g) {
            // we found a better g, update the parent of the adjacent node
            open_node->parent = std::make_shared<SearchState>(new_node);
            // recalculate f g h
            open_node->g = g_count;
            open_node->f = open_node->g + open_node->h;
          }
        } else {
          // did not already exist in the open list, so insert it
          pq.push(new_node);
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

  std::cout << "No path found\n";
  return {};
}
