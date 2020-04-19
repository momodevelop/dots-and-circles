#For now we cater only for VS x64 and x32

BUILD_DIR=__build

BUILD_DIR_X64=$(BUILD_DIR)/x64
BUILD_DIR_WIN32=$(BUILD_DIR)/Win32

CONFIG_DEBUG=Debug
CONFIG_RELEASE=Release

TOOLCHAIN_X64=cmake/toolchain_x64.cmake
TOOLCHAIN_WIN32=cmake/toolchain_win32.cmake

LIB_DIR=lib

init: init-vs
init-vs: init-x64 init-x86
init-x64:
	mkdir -p $(BUILD_DIR) && mkdir -p $(BUILD_DIR_X64) && cd $(BUILD_DIR_X64) && cmake -DCMAKE_TOOLCHAIN_FILE=../../$(TOOLCHAIN_WIN32) -S ../..
init-x86:	
	mkdir -p $(BUILD_DIR) && mkdir -p $(BUILD_DIR_WIN32) && cd $(BUILD_DIR_WIN32) && cmake -DCMAKE_TOOLCHAIN_FILE=../../$(TOOLCHAIN_X64) -S ../..

build: build-debug build-release
build-debug: build-debug-x86 build-debug-x64
build-release: build-release-x86 build-release-x64
build-debug-x86:
	cd $(BUILD_DIR_WIN32) && cmake --build . --config $(CONFIG_DEBUG)	
build-debug-x64:
	cd $(BUILD_DIR_X64) && cmake --build . --config $(CONFIG_DEBUG)
build-release-x86:
	cd $(BUILD_DIR_WIN32) && cmake --build . --config $(CONFIG_RELEASE)	
build-release-x64:
	cd $(BUILD_DIR_X64) && cmake --build . --config $(CONFIG_RELEASE)

clean: clean-vs
clean-vs: clean-x86 clean-x64
clean-x86:
	rm -rf ${BUILD_DIR_WIN32}/*
clean-x64:
	rm -rf ${BUILD_DIR_X64}/*