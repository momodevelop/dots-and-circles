#ifndef __APP_H__
#define __APP_H__

#include "yuu/inputs/keyboard.h"


class App {
	using Keyboard = yuu::input::Keyboard;

private:
	Keyboard kb;

public:
	App();
	~App();

	void run();

};


#endif