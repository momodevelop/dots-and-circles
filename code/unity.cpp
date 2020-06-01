void PrintGoodOrBad(bool good) {
	if (good)
		cout << "[" << ANSI_COLOR_GREEN << "O" << ANSI_COLOR_RESET << "] ";
	else
		cout << "[" << ANSI_COLOR_RED << "X" << ANSI_COLOR_RESET << "] ";

}

#include "units/test-ds.cpp"
#include "units/test-maths.cpp"
#include "units/test-utils.cpp"
#include "main.cpp"


