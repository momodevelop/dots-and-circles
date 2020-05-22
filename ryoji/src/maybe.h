#ifndef __RYOJI_MAYBE_H__
#define __RYOJI_MAYBE_H__

#include <assert.h>

namespace ryoji {
	namespace maybe {
		template<typename T>
		struct Maybe {
			T item;
			bool isNothing;
		};

		template<typename T>
		inline Maybe<T> make() {
			return { T(), true };
		}

		template<typename T>
		inline Maybe<T> make(const T& item) {
			return { item, false };
		}

		template<typename T>
		inline bool isYes(const Maybe<T>& lhs) {
			return !lhs.isNothing;
		}

		template<typename T>
		inline bool isNo(const Maybe<T>& lhs) {
			return lhs.isNothing;
		}

		template<typename T>
		inline T& get(Maybe<T>& lhs) {
			assert(!lhs.isNothing);
			return lhs.item;
		}
	}

	using maybe::Maybe;
}
#endif