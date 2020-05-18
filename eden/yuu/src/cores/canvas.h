#ifndef __YUU_CORE_CANVAS_H__
#define __YUU_CORE_CANVAS_H__

#include <string>
#include <optional>
#include "ryoji/utils/expected.h"

union SDL_Event;
using Event = SDL_Event;

namespace yuu::cores {
	namespace canvases {
		
		// Only responsible of wrapping SDL2 settings
		// Sadly, should be a singleton
		class Canvas {
			using Error = const char*;
			template<typename T> using Expect = ryoji::Expected<T, Error>;
			inline static bool initialized = false;
			inline static bool constructed = false;

			Canvas(const Canvas&) = delete;
			Canvas& operator=(const Canvas&) = delete;

		public:
			Canvas();
			~Canvas();

			Expect<void> init();
			void free();

			std::optional<SDL_Event> pollEvent();

			bool isValid() const;

		};

	}

	using canvases::Canvas;
}


#endif