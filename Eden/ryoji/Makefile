#For now we cater only for VS x64 and x32

BUILD_DIR=__build

BUILD_DIR_X64=$(BUILD_DIR)/x64
BUILD_DIR_WIN32=$(BUILD_DIR)/Win32

CONFIG_DEBUG=Debug
CONFIG_RELEASE=Release

TOOLCHAIN_X64=cmake/toolchain_x64.cmake
TOOLCHAIN_WIN32=cmake/toolchain_win32.cmake

EXE_DIR=__bin

default: init build

init: init-vs
init-vs: init-x64 init-win32
init-x64:
	mkdir -p $(BUILD_DIR) && mkdir -p $(BUILD_DIR_X64) && cd $(BUILD_DIR_X64) && cmake -DCMAKE_TOOLCHAIN_FILE=../../$(TOOLCHAIN_X64) -S ../..
init-win32:	
	mkdir -p $(BUILD_DIR) && mkdir -p $(BUILD_DIR_WIN32) && cd $(BUILD_DIR_WIN32) && cmake -DCMAKE_TOOLCHAIN_FILE=../../$(TOOLCHAIN_WIN32) -S ../..

build: build-debug build-release
build-debug: build-debug-win32 build-debug-x64
build-release: build-release-win32 build-release-x64
build-x64: build-debug-x64 build-release-x64
build-win32: build-debug-win32 build-release-win32
build-debug-win32:
	cd $(BUILD_DIR_WIN32) && cmake --build . --config $(CONFIG_DEBUG)	
build-debug-x64:
	cd $(BUILD_DIR_X64) && cmake --build . --config $(CONFIG_DEBUG)
build-release-win32:
	cd $(BUILD_DIR_WIN32) && cmake --build . --config $(CONFIG_RELEASE)	
build-release-x64:
	cd $(BUILD_DIR_X64) && cmake --build . --config $(CONFIG_RELEASE)

clean: clean-vs
clean-vs: clean-win32 clean-x64
clean-win32:
	rm -rf ${BUILD_DIR_WIN32}/*
clean-x64:
	rm -rf ${BUILD_DIR_X64}/*