#include <iostream>
#include "ryoji/defer.h"
#include "ryoji/reflection.h"
#include "ryoji/either.h"

using namespace std;
using namespace ryoji;
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

namespace {
	void PrintGoodOrBad(bool good) {
		if (good)
			cout << "[" << ANSI_COLOR_GREEN << "O" << ANSI_COLOR_RESET << "] ";
		else
			cout << "[" << ANSI_COLOR_RED << "X" << ANSI_COLOR_RESET << "] ";

	}

	namespace test_defer {
		int gDeferSuccesses = 0;
		constexpr int gDeferSuccessesRequired = 2;

		void simpleFunction() {
			cout << "[defer] Simple function" << endl;
			gDeferSuccesses++;
		}
		void simpleFunctionWithParams(int x) {
			cout << "[defer] Simple function with params (x = " << x << ")" << endl;
			gDeferSuccesses++;
		}
		void TestDeferSub() {

			struct DeferFoo {
				int x = 0;
				void memberFunction() {
					++x;
					cout << "[defer] Member function (x = " << this->x << ")" << endl;
				}
			};


			cout << "=== Testing defer" << endl;
			DeferFoo foo;
			defer{
				simpleFunction();
			};

			int x = 10;
			defer{
				simpleFunctionWithParams(x);
			};

			defer{
				foo.memberFunction();
			};


			if (true) {
				return;
			}
			cout << "[" << ANSI_COLOR_RED << "X" << ANSI_COLOR_RESET << "] This line should not be called!" << endl;
			return;
		}
		void Test() {
			gDeferSuccesses = 0;
			TestDeferSub();
			PrintGoodOrBad(gDeferSuccessesRequired == gDeferSuccesses);
			cout << "Are defers working (" << gDeferSuccesses << "/" << gDeferSuccessesRequired << ")" << endl;
			cout << endl;
		}
	}


	namespace test_reflection {

		struct Karu {};
		struct Sui {
			void foo(int);
		};
		GENERATE_MEMBER_FUNCTION_PRODDER(has_, foo, void, (int));

		void Test() {
			{
				cout << "=== Testing member function prodder" << endl;
				PrintGoodOrBad(has_foo_v<Karu> == false);
				cout << "Struct should not have member function" << endl;
				PrintGoodOrBad(has_foo_v<Sui> == true);
				cout << "Struct should have member function" << endl;
			}
			cout << endl;
		}
	}

	namespace test_either {
		
		enum struct FailReasons {
			TILTED
		};

		struct Value {
			int value = 10;
		};
		#define RetType Value,FailReasons


		Either<RetType> eitherTest(bool tilt) {
			using namespace either;
			if (tilt)
				return make<RetType>(FailReasons::TILTED) ;
			return make<RetType>(Value());
		}

		Either<RetType> eitherVoidTest(bool tilt) {
			using namespace either;
			if (tilt)
				return make<RetType>(FailReasons::TILTED);
			return make<RetType>(Value());
		}

		void Test() {
			using namespace either;
			cout << "=== Testing Either" << endl;
			{
				auto ok = eitherTest(true);
				PrintGoodOrBad(isRight(ok));
				cout << "Either should be right" << endl;
				PrintGoodOrBad(ok.right == FailReasons::TILTED);
				cout << "Either error value is correct" << endl;
				PrintGoodOrBad(getRight(ok) == FailReasons::TILTED);
				cout << "Either error value from get is correct" << endl;
			}
			{
				auto ok = eitherTest(false);
				PrintGoodOrBad(isLeft(ok));
				cout << "Either should be either" << endl;
				PrintGoodOrBad(ok.left.value == 10);
				cout << "Expect value is correct" << endl;
			}
			{
				auto ok = eitherVoidTest(true);
				PrintGoodOrBad(isRight(ok));
				cout << "Expect<void> should be uneither" << endl;
				PrintGoodOrBad(ok.right == FailReasons::TILTED);
				cout << "Expect<void> error value is correct" << endl;
			}
			{
				auto ok = eitherVoidTest(false);
				PrintGoodOrBad(isLeft(ok));
				cout << "Expect<void> should be either" << endl;
			}

		}
	}
}


int TestUtils() {
	test_defer::Test();
	test_reflection::Test();
	test_either::Test();
	return 0;
}

#undef CONCAT