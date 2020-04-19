#For now we cater only for VS x64 and x32

BUILD_DIR_BASE=__build
VS_PLATFORM_X86=Win32
VS_PLATFORM_X64=x64
VS_BUILD_DIR_X64=$(BUILD_DIR_BASE)/vs_x64
VS_BUILD_DIR_X86=$(BUILD_DIR_BASE)/vs_x86
VS_GENERATOR="Visual Studio 16 2019"
VS_DEBUG_CONFIG=Debug
VS_RELEASE_CONFIG=Release

init: init-vs
init-vs: init-vs-x64 init-vs-x86
init-vs-x64:
	mkdir -p $(BUILD_DIR_BASE) && mkdir -p $(VS_BUILD_DIR_X64) && cd $(VS_BUILD_DIR_X64) && cmake -G $(VS_GENERATOR) -A $(VS_PLATFORM_X64) -S ../..
init-vs-x86:	
	mkdir -p $(BUILD_DIR_BASE) && mkdir -p $(VS_BUILD_DIR_X86) && cd $(VS_BUILD_DIR_X86) && cmake -G $(VS_GENERATOR) -A $(VS_PLATFORM_X86) -S ../..

build: build-vs-debug build-vs-release
build-vs-debug: build-vs-debug-x86 build-vs-debug-x64
build-vs-release: build-vs-release-x86 build-vs-release-x64
build-vs-debug-x86:
	cd $(VS_BUILD_DIR_X86) && cmake --build . --config $(VS_DEBUG_CONFIG)	
build-vs-debug-x64:
	cd $(VS_BUILD_DIR_X64) && cmake --build . --config $(VS_DEBUG_CONFIG)
build-vs-release-x86:
	cd $(VS_BUILD_DIR_X86) && cmake --build . --config $(VS_RELEASE_CONFIG)	
build-vs-release-x64:
	cd $(VS_BUILD_DIR_X64) && cmake --build . --config $(VS_RELEASE_CONFIG)


clean: clean-vs
clean-vs: clean-vs-x86 clean-vs-x64
clean-vs-x86:
	rm -rf ${VS_BUILD_DIR_X86}/*
clean-vs-x64:
	rm -rf ${VS_BUILD_DIR_X64}/*