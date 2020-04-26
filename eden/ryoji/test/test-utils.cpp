#include <iostream>
#include "ryoji/utils/defer.h"
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

void TestDefer() {
	gDeferSuccesses = 0;
	TestDeferSub();
	PrintGoodOrBad(gDeferSuccessesRequired == gDeferSuccesses);
	cout << "Are defers working (" << gDeferSuccesses << "/" << gDeferSuccessesRequired << ")" << endl;
}

int main() {
	TestDefer();
	return 0;
}

#undef CONCAT