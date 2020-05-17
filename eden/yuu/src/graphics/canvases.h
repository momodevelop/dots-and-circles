#ifndef __YUU_GRAPHICS_CANVASES_H__
#define __YUU_GRAPHICS_CANVASES_H__

#include <string>
#include <optional>
#include "ryoji/utils/expected.h"

union SDL_Event;
using Event = SDL_Event;

namespace yuu::graphics {
	namespace canvases {
		
		// Only responsible of wrapping SDL2 settings
		class Canvas {
			struct Error {};
			template<typename T> using Expect = ryoji::Expected<T, Error>;


			std::string lastError{};

			#ifdef _DEBUG
				inline static bool initialized = false;
			#endif
		public:
			Canvas();
			~Canvas();

			Expect<void> init();
			void free();

			const char* getLastError() const;

			// Wrapper to change the awkward way SDL_PollEvent is being called.
			std::optional<SDL_Event> pollEvent();

			#ifdef _DEBUG
				bool isInitialized() const;
			#endif

		};

	}

	using canvases::Canvas;
}


#endif