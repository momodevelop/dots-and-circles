void PrintGoodOrBad(bool good) {
	if (good)
		cout << "[" << ANSI_COLOR_GREEN << "O" << ANSI_COLOR_RESET << "] ";
	else
		cout << "[" << ANSI_COLOR_RED << "X" << ANSI_COLOR_RESET << "] ";

}
	
int main() {
#if 0
	TestAllocators();
#endif
#if 0
	TestMaths();
#endif
#if 0
	TestUtils();
#endif
#if 1
	TestDS();
#endif

	return 0;
}