@echo off

SET RootDir=%me%..
SET BuildDir=%RootDir%\build

pushd %BuildDir%

call sdl_platform.exe

popd