#ifndef __APP_H__
#define __APP_H__

#include "ryoji/allocators/linear-allocator.h"
#include "yuu/cores/window.h"

class App {
	using Allocator = ryoji::allocators::LocalLinearAllocator<256>;
	using Window = yuu::cores::Window;

	Allocator allocator;
	std::shared_ptr<Window> window;
	
public:
	App();
	~App();


};


#endif