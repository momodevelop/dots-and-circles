@echo off
SET me=%~dp0

SET RootDir=%me%..
SET CodeDir=%RootDir%\code
SET BuildDir=%RootDir%\build
SET Opt=%1%

IF NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (EXIT /b)

call %me%..\bin\setup_cl_x64.bat


SET CommonCompilerFlags= -MT -WX -W4 -wd4702 -wd4201 -wd4505 -wd4996 -wd4100 -Zi -Oi -GR- -EHa -Gm- -std:c++17
SET CommonCompilerFlags=-DSLOW -DINTERNAL  %CommonCompilerFlags%

SET CommonLinkerFlags=-incremental:no -subsystem:console -libpath:%CodeDir%\thirdparty\sdl2\lib\x64
SET CommonLinkerFlags=SDL2main.lib SDL2.lib shell32.lib opengl32.lib %CommonLinkerFlags%

SET common_flags=%opts% %additional% %entry_point% %include_dir% -link %libs% %linker_opts% 

pushd %BuildDir%

copy %CodeDir%\thirdparty\sdl2\lib\x64\SDL2.dll %cd%

rmdir %cd%\assets
mkdir %cd%\assets
copy %RootDir%\assets\* %cd%\assets

IF "%Opt%"=="game" (
	cl %CommonCompilerFlags% %CodeDir%\game.cpp -LD -link -EXPORT:GameUpdate
) ELSE (
	cl %CommonCompilerFlags% %CodeDir%\game.cpp -LD -link -EXPORT:GameUpdate
	cl %CommonCompilerFlags%  %CodeDir%\sdl_platform.cpp  -link %CommonLinkerFlags%
)

popd