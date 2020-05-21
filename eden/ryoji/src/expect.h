#ifndef __RYOJI_EXPECT_H__
#define __RYOJI_EXPECT_H__

#include <utility>

namespace ryoji {
	namespace expect {
		// Note: The reason why we use constructors for this case
		// is because there is a valid/invalid state for this struct on creation...
		// If only we can create constructors outside the the struct though...
		// I guess this is a bit of a special case because I'm dealing with unions.
		//
		template<typename Value, typename Error>
		struct Expect {
			union {
				Value value;
				Error error;
			};
			bool isError = false;
			
			// We need placement new to emulate member initialization schementics...
			// because we can't do this correctly in member initialization step...
			Expect(const Expect& rhs) : isError(rhs.isError) {
				isError ? new(&error) Error(rhs.error) : new(&value) Value(rhs.value);
			}
			Expect(Expect&& rhs) : isError(rhs.isError) {
				isError ? new(&error) Error(std::move(rhs.error)) : new(&value) Value(std::move(rhs.value));
			}
			~Expect() {
				isError ? error.~Error() : value.~Value();
			}
			
			Expect(const Value& rhs) : value(rhs), isError(false) {}
			Expect(const Error& rhs) : error(rhs), isError(true) {}
			Expect(Value&& rhs) : value(std::move(rhs)), isError(false) {}
			Expect(Error&& rhs) : error(std::move(rhs)), isError(true) {}

			inline operator bool() const noexcept { return !isError; }

			// Note: I have thought about writing accessors to 'value' and 'error' that will panic and die
			// when, say, retrieving 'error' when 'isError' is false. But seeing that there is already a way
			// to check via 'isError', I don't feel that creating a function to check again is helpful.
			// I could leave it up to extension outside of the class. I think that makes sense :)
		};


		template<typename Error>
		struct Expect<void, Error> {
			Error error;
			bool isError = false;

			Expect(const Error& rhs) : error(rhs), isError(true) {}
			Expect(Error&& rhs) : error(std::move(rhs)) {}
			Expect(const Expect& rhs) : isError(rhs.isError) {
				if (!isError) new(&error) Error(rhs.error)
			}
			Expect(Expect&& rhs) : isError(rhs.isError) {
				if (!isError) new(&error) Error(std::move(rhs.error))
			}
			~Expect() {
				if (!isError) error.~Error();
			}

			inline operator bool() const noexcept{ return !isError; }
		};

		template<typename UnexpectedType>
		class Unexpected {
			UnexpectedType value;
			Unexpected() = default;
			Unexpected(const UnexpectedType& value) noexcept : value(value) {}
		};
	}
	using expect::Expect;
	using expect::Unexpected;


}

#endif