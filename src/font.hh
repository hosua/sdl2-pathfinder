#ifdef __EMSCRIPTEN__
#define FONT_OPENSANS "/assets/fonts/OpenSans-Regular.ttf"
#else
#define FONT_OPENSANS "./assets/fonts/OpenSans-Regular.ttf"
#endif

#include <SDL2/SDL_ttf.h>

namespace Font {
	extern TTF_Font* openSansTiny;
	extern TTF_Font* openSansSmall;
	extern TTF_Font* openSansMedium;
	extern TTF_Font* openSansLarge;

	bool init(); // return false on failure
	void close();
}

