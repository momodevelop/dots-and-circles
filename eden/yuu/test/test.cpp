#include <iostream>
#include "yuu/utils/window.h"
#include "ryoji/utils/defer.h"

int main(int argc, char* args[]){
	using namespace yuu::utils;
	
	Window window;
	window.init("Yuu Test");
	defer{ window.free(); };





	return 0;
}