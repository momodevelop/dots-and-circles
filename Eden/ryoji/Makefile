#For now we cater only for VS x64 and x32

BUILD_DIR=__build


BUILD_DIR_DEBUG_X64=$(BUILD_DIR)/Debug/x64
BUILD_DIR_DEBUG_WIN32=$(BUILD_DIR)/Debug/Win32
BUILD_DIR_RELEASE_X64=$(BUILD_DIR)/Release/x64
BUILD_DIR_RELEASE_WIN32=$(BUILD_DIR)/Release/Win32



TOOLCHAIN_DEBUG_X64=cmake/debug_x64.cmake
TOOLCHAIN_DEBUG_WIN32=cmake/debug_win32.cmake
TOOLCHAIN_RELEASE_X64=cmake/release_x64.cmake
TOOLCHAIN_RELEASE_WIN32=cmake/release_win32.cmake

default: init build

init: init-x64 init-win32
init-x64: init-debug-x64 init-release-x64
init-win32: init-debug-win32 init-release-win32
init-debug: init-debug-x64 init-debug-win32
init-release: init-release-x64 init-release-win32
init-debug-x64:
	mkdir -p $(BUILD_DIR) && mkdir -p $(BUILD_DIR_DEBUG_X64) && cd $(BUILD_DIR_DEBUG_X64) && cmake -DCMAKE_TOOLCHAIN_FILE=../../../$(TOOLCHAIN_DEBUG_X64) -S ../../..
init-debug-win32:	
	mkdir -p $(BUILD_DIR) && mkdir -p $(BUILD_DIR_DEBUG_WIN32) && cd $(BUILD_DIR_DEBUG_WIN32) && cmake -DCMAKE_TOOLCHAIN_FILE=../../../$(TOOLCHAIN_DEBUG_WIN32) -S ../../..
init-release-x64:
	mkdir -p $(BUILD_DIR) && mkdir -p $(BUILD_DIR_RELEASE_X64) && cd $(BUILD_DIR_RELEASE_X64) && cmake -DCMAKE_TOOLCHAIN_FILE=../../../$(TOOLCHAIN_RELEASE_X64) -S ../../..
init-release-win32:	
	mkdir -p $(BUILD_DIR) && mkdir -p $(BUILD_DIR_RELEASE_WIN32) && cd $(BUILD_DIR_RELEASE_WIN32) && cmake -DCMAKE_TOOLCHAIN_FILE=../../../$(TOOLCHAIN_RELEASE_WIN32) -S ../../..


build: build-x64 build-win32
build-debug: build-debug-win32 build-debug-x64
build-release: build-release-win32 build-release-x64
build-x64: build-debug-x64 build-release-x64
build-win32: build-debug-win32 build-release-win32
build-debug-win32:
	cd $(BUILD_DIR_DEBUG_WIN32) && cmake --build .
build-debug-x64:
	cd $(BUILD_DIR_DEBUG_X64) && cmake --build .
build-release-win32:
	cd $(BUILD_DIR_RELEASE_WIN32) && cmake --build . 
build-release-x64:
	cd $(BUILD_DIR_RELEASE_X64) && cmake --build . 

clean: clean-win32 clean-x64
clean-debug: clean-debug-win32 clean-debug-x64
clean-release: clean-release-win32 clean-release-x64
clean-x64: clean-debug-x64 clean-release-x64
clean-win32: clean-debug-win32 clean-release-win32
clean-debug-win32:
	rm -rf ./$(BUILD_DIR_DEBUG_WIN32)/*
clean-debug-x64:
	rm -rf ./$(BUILD_DIR_DEBUG_X64)/*
clean-release-win32:
	rm -rf ./$(BUILD_DIR_RELEASE_WIN32)/*
clean-release-x64:
	rm -rf ./$(BUILD_DIR_RELEASE_X64)/*
