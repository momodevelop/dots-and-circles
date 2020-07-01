@echo off

SET me=%~dp0

SET CodeDir=%me%..\code

IF NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (EXIT /b)

call %me%..\bin\setup_cl_x64.bat


SET CommonCompilerFlags= -MT -WX -W4 -wd4201 -wd4505 -wd4996 -wd4100 -Zi -Oi -GR- -EHa -Gm- -std:c++17
SET CommonCompilerFlags=-DSLOW_MODE  %CommonCompilerFlags%

SET CommonLinkerFlags=-incremental:no -subsystem:console -libpath:%CodeDir%\thirdparty\sdl2\lib\x64
SET CommonLinkerFlags=SDL2main.lib SDL2.lib shell32.lib opengl32.lib %CommonLinkerFlags%

SET common_flags=%opts% %additional% %entry_point% %include_dir% -link %libs% %linker_opts% 
copy %CodeDir%\thirdparty\sdl2\lib\x64\SDL2.dll %cd%

rmdir %cd%\shader
mkdir %cd%\shader
copy ..\shader\* %cd%\shader

rmdir %cd%\assets
mkdir %cd%\assets
copy ..\assets\* %cd%\assets


cl %CommonCompilerFlags% %CodeDir%\game.cpp -LD -link -EXPORT:GameUpdate
cl %CommonCompilerFlags%  %CodeDir%\sdl_platform.cpp  -link %CommonLinkerFlags%
