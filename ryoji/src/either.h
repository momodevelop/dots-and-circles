#ifndef __RYOJI_EITHER_H__
#define __RYOJI_EITHER_H__

#include <assert.h>
namespace ryoji {

	namespace either {
		template<typename Left, typename Right>
		struct Either {
			union {
				Left left;
				Right right;
			};
			bool isRight;
		};

		template<typename Right>
		struct Either<void, Right> {
			Right right;
			bool isRight = false;
		};

		template<typename L, typename R>
		inline Either<L, R> make(const L& lhs) {
			return { {.left = lhs }, false };
		}

		template<typename L, typename R>
		inline Either<L, R> make(const R& rhs) {
			return { {.right = rhs }, true };
		}

		template<typename L, typename R>
		inline bool isLeft(const Either<L, R>& lhs) {
			return !lhs.isRight;
		}

		template<typename L, typename R>
		inline bool isRight(const Either<L, R>& lhs) {
			return lhs.isRight;
		}

		template<typename L, typename R>
		inline L& getLeft(Either<L, R>& lhs) {
			assert(!lhs.isRight);
			return lhs.left;
		}

		template<typename L, typename R>
		inline R& getRight(Either<L, R>& lhs) {
			assert(lhs.isRight);
			return lhs.right;
		}
	}
	using either::Either;

}

#endif