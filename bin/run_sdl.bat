@echo off

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET Opt=%1%

pushd %BuildDir%

call platform_sdl_opengl.exe

:End

popd
