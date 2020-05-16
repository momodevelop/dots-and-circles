#ifndef __YUU_GRAPHICS_ERRORS_H__
#define __YUU_GRAPHICS_ERRORS_H__

namespace yuu::graphics {
	namespace error {
		enum Error  {
			CANNOT_INIT_SDL,
			WINDOW_ALREADY_CREATED,
		};

		const char* getErrorString(Error err) {
			switch (err) {
			case CANNOT_INIT_SDL:
				return "Cannot init SDL";
			}

			return nullptr;

		}

	}
}

#endif