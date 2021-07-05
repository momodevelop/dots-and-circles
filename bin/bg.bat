@echo off

SET me=%~dp0

SET RootDir=%me%..
SET CodeDir=%RootDir%\code
SET BuildDir=%RootDir%\build
SET Opt=%1

if not exist %BuildDir% mkdir %BuildDir%

call bin\setup_cl_x64.bat
IF NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (EXIT /b)


SET CommonCompilerFlags=-MT -WX -W4 -wd4189 -wd4702 -wd4201 -wd4505 -wd4996 -wd4100 -Zi  -GR -EHa  -std:c++17
SET CommonCompilerFlags=-DSLOW -DINTERNAL  %CommonCompilerFlags%

pushd %BuildDir%
cl %CommonCompilerFlags% %CodeDir%\game.cxx -LD -link -EXPORT:game_update

rem We do this because for some reason, compiling will result in the DLL being modified twice and cus our hot reloading feature to be loaded twice. This *should* prevent it.

copy game.dll compiled_game.dll
del game.dll

popd
