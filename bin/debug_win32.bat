@echo off

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET Opt=%1%

pushd %BuildDir%

call devenv platform_win32_opengl.exe

:End 

popd