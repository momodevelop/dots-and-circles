@echo off

SET RootDir=%me%..
SET BuildDir=%RootDir%\build

pushd %BuildDir%

call devenv sdl_platform.exe

popd