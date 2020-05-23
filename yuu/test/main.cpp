#include <iostream>
#include <optional>

#include "app.h"
#include "ryoji/defer.h"


int main(int argc, char* args[]){
	App app;

	
	if (!init(app)) 
		return 1;
	defer{ free(app); };
	
	
	run(app);
	return 0;
}