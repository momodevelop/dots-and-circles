#include <iostream>
#include "ryoji/utils/defer.h"
#include "ryoji/utils/reflection.h"
using namespace std;

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

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

	struct Karu{};
	struct Sui {
		void foo(int);
	};
	GENERATE_MEMBER_FUNCTION_PRODDER(has_, foo, void, (int))

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

int main() {
	test_defer::Test();
	test_reflection::Test();
	return 0;
}

#undef CONCAT