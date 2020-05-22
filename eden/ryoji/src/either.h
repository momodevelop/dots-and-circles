#ifndef __RYOJI_EITHER_H__
#define __RYOJI_EITHER_H__

#include <utility>

namespace ryoji {
	namespace either {
		// TODO: What if left and right are same type...?
		// - Do we Either users to wrap one of the type with another type?

		// Note: Why we use constructors/destructors
		// - There exist a valid/invalid state for this struct on creation.
		// - Special case because we are dealing with union
		// - There are type conversations that I want to be implicit
		//
		template<typename Left, typename Right>
		struct Either {
			union {
				Left left;
				Right right;
			};
			bool isRight = false;
			
			// We need placement new to emulate member initialization schementics...
			// because we can't do this correctly in member initialization step...
			Either(const Either& rhs) : isRight(rhs.isRight) {
				isRight ? new(&right) right(rhs.right) : new(&left) left(rhs.left);
			}
			Either(Either&& rhs) : isRight(rhs.isRight) {
				isRight ? new(&right) right(std::move(rhs.right)) : new(&left) left(std::move(rhs.left));
			}
			~Either() {
				isRight ? right.~Right() : left.~Left();
			}
			
			Either(const Left& rhs) : left(rhs), isRight(false) {}
			Either(const Right& rhs) : right(rhs), isRight(true) {}
			Either(Left&& rhs) : left(std::move(rhs)), isRight(false) {}
			Either(Right&& rhs) : right(std::move(rhs)), isRight(true) {}

			inline explicit operator bool() const noexcept { return !isRight; }

			// Note: I have thought about writing accessors to 'left' and 'right' that will panic and die
			// when, say, retrieving 'right' when 'isRight' is false. But seeing that there is already a way
			// to check via 'isRight', I don't feel that creating a function to check again is helpful.
			// I could leave it up to extension outside of the class. I think that makes sense :)
		};


		template<typename Right>
		struct Either<void, Right> {
			Right right;
			bool isRight = false;

			Either(const Right& rhs) : right(rhs), isRight(true) {}
			Either(Right&& rhs) : right(std::move(rhs)) {}
			Either(const Either& rhs) : isRight(rhs.isRight) {
				if (!isRight) new(&right) right(rhs.right)
			}
			Either(Either&& rhs) : isRight(rhs.isRight) {
				if (!isRight) new(&right) right(std::move(rhs.right))
			}
			~Either() {
				if (!isRight) right.~right();
			}

			inline operator bool() const noexcept{ return !isRight; }
		};

	}
	using either::Either;


}

#endif