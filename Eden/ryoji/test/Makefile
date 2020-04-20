#For now we cater only for VS win64 and x32

BUILD_DIR=__build


BUILD_DIR_WIN64=$(BUILD_DIR)/win64
BUILD_DIR_WIN32=$(BUILD_DIR)/win32

TOOLCHAIN_WIN64=cmake/win64.cmake
TOOLCHAIN_WIN32=cmake/win32.cmake

default: init build

init: init-win64 init-win32
init-win64: init-debug-win64 init-release-win64
init-win32: init-debug-win32 init-release-win32
init-debug: init-debug-win64 init-debug-win32
init-release: init-release-win64 init-release-win32
init-debug-win64:
	mkdir -p $(BUILD_DIR) && \
	mkdir -p $(BUILD_DIR_WIN64) && \
	cd $(BUILD_DIR_WIN64) && \
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../$(TOOLCHAIN_WIN64) -S ../../.. 
init-debug-win32:	
	mkdir -p $(BUILD_DIR) && \
	mkdir -p $(BUILD_DIR_DEBUG_WIN32) && \
	cd $(BUILD_DIR_DEBUG_WIN32) && \
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../$(TOOLCHAIN_WIN32) -S ../../.. -A Debug
init-release-win64:
	mkdir -p $(BUILD_DIR) && \
	mkdir -p $(BUILD_DIR_WIN64) && \
	cd $(BUILD_DIR_WIN64) && \
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../$(TOOLCHAIN_WIN64) -S ../../.. -A Release
init-release-win32:	
	mkdir -p $(BUILD_DIR) && \
	mkdir -p $(TOOLCHAIN_WIN32) && \
	cd $(TOOLCHAIN_WIN32) && \
	cmake -DCMAKE_TOOLCHAIN_FILE=../../../$(TOOLCHAIN_WIN32) -S ../../..


build: build-win64 build-win32
build-debug: build-debug-win32 build-debug-win64
build-release: build-release-win32 build-release-win64
build-win64: build-debug-win64 build-release-win64
build-win32: build-debug-win32 build-release-win32
build-debug-win32:
	cd $(BUILD_DIR_WIN32) && cmake --build . -A Debug
build-debug-win64:
	cd $(BUILD_DIR_WIN64) && cmake --build . -A Debug
build-release-win32:
	cd $(BUILD_DIR_WIN32) && cmake --build . -A Release
build-release-win64:
	cd $(BUILD_DIR_WIN64) && cmake --build . -A Release 

clean: clean-win32 clean-win64
clean-debug: clean-debug-win32 clean-debug-win64
clean-release: clean-release-win32 clean-release-win64
clean-win64: clean-debug-win64 clean-release-win64
clean-win32: clean-debug-win32 clean-release-win32
clean-win32:
	rm -rf ./$(BUILD_DIR_WIN32)/*
clean-win64:
	rm -rf ./$(BUILD_DIR_DEBUG_X64)/*
clean-win32:
	rm -rf ./$(BUILD_DIR_WIN32)/*
clean-win64:
	rm -rf ./$(BUILD_DIR_RELEASE_X64)/*
