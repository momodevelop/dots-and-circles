#ifndef __YUU_GRAPHICS_CANVAS_H__
#define __YUU_GRAPHICS_CANVAS_H__

#include <string>
#include "ryoji/utils/expected.h"
#include "window.h"

namespace yuu::graphics {

	namespace canvas {
		// Only responsible managing SDL2 settings
		class Canvas {
			struct Error {};
			template<typename T> using Expect = ryoji::Expected<T, Error>;

			bool initialized{ false };
			std::string lastError{};
		public:
			Canvas();
			~Canvas();

			Expect<void> init();
			Expect<void> free();

			const char* getLastError() const;
			bool isInitialized() const;

		};

	}

	using canvas::Canvas;
}


#endif