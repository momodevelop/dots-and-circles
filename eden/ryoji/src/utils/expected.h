#ifndef __RYOJI_EXPECTED_H__
#define __RYOJI_EXPECTED_H__

#include <assert.h>

namespace ryoji {
	namespace expected {
		template<typename Value, typename Error>
		class Expected {
			union {
				Value ham;
				Error spam;
			};
			bool gotHam{ false };
			Expected() = delete;
		public:
			Expected(const Value& rhs) : ham(rhs), gotHam(true) {}
			Expected(Value&& rhs) : ham(std::move(rhs)), gotHam(true) {}
			Expected(const Error& rhs) : spam(rhs) {}
			Expected(Error&& rhs) : spam(move(rhs)) {}
			Expected(const Expected& rhs) : gotHam(rhs.gotHam) {
				if (gotHam)
					new(&ham) Value(rhs.ham);
				else
					new(&spam) Error(rhs.spam);
			}
			Expected(Expected&& rhs) : gotHam(rhs.gotHam) {
				if (gotHam)
					new(&ham) Value(std::move(rhs.ham));
				else
					new(&spam) Error(std::move(rhs.spam));
			}

			~Expected() {
				if (gotHam)
					ham.~Value();
				else
					spam.~Error();
			}

			inline operator bool() const noexcept {
				return gotHam;
			}

			Value& getValue() noexcept {
				assert(gotHam);
				return ham;
			}

			const Value& getValue() const noexcept {
				assert(gotHam);
				return ham;
			}

			Error& getError() noexcept {
				assert(!gotHam);
				return spam;
			}

			const Error& getError() const noexcept {
				assert(!gotHam);
				return spam;
			}
		};


		template<typename Error>
		class Expected<void, Error> {
			Error spam;
			bool gotHam{ false };
		public:
			Expected() : gotHam(true) {}
			Expected(const Error& rhs) : spam(rhs) {}
			Expected(Error&& rhs) : spam(std::move(rhs)) {}
			Expected(const Expected& rhs) : gotHam(rhs.gotHam) {
				if (!gotHam)
					new(&spam) Error(rhs.spam);
			}
			Expected(Expected&& rhs) : gotHam(rhs.gotHam) {
				if (!gotHam)
					new(&spam) Error(std::move(rhs.spam));
			}

			~Expected() {
				if (!gotHam)
					spam.~Error();
			}

			inline operator bool() const noexcept {
				return gotHam;
			}

			Error& getError() noexcept {
				assert(!gotHam);
				return spam;
			}

			const Error& getError() const noexcept  {
				assert(!gotHam);
				return spam;
			}
		};

		template<typename UnexpectedType>
		class Unexpected {
			UnexpectedType value;
		public:
			Unexpected() = default;
			Unexpected(const UnexpectedType& value) noexcept : value(value) {}
			inline UnexpectedType& getValue() noexcept { return value; }
			inline const UnexpectedType& getValue() const noexcept { return value; }
		};
	}
	using expected::Expected;
	using expected::Unexpected;


}

#endif