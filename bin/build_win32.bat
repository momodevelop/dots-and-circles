@echo off
SET me=%~dp0

SET RootDir=%me%..
SET CodeDir=%RootDir%\code
SET BuildDir=%RootDir%\build
SET Opt=%1

IF NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (EXIT /b)
call %me%..\bin\setup_cl_x64.bat

SET CommonCompilerFlags=-MT -WX -W4 -wd4189 -wd4702 -wd4201 -wd4505 -wd4996 -wd4100 -Zi -Oi -GR- -EHa -Gm- -std:c++17
SET CommonCompilerFlags=-DSLOW -DINTERNAL  %CommonCompilerFlags%

SET CommonLinkerFlags=-incremental:no -opt:ref
SET CommonLinkerFlags=user32.lib opengl32.lib gdi32.lib winmm.lib %CommonLinkerFlags%


pushd %BuildDir%

cl %CommonCompilerFlags% %CodeDir%\game.cpp -LD -link -EXPORT:GameUpdate
cl %CommonCompilerFlags% %CodeDir%\target_win32_opengl.cpp -link %CommonLinkerFlags%


:End

popd
