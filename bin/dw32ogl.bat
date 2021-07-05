@echo off
SET me=%~dp0
SET RootDir=%me%\..
SET BuildDir=%RootDir%\build
SET Opt=%1%


call bin\setup_cl_x64.bat
IF NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (EXIT /b)

pushd %BuildDir%

call devenv target_win32_opengl.exe

:End 

popd
