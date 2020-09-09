@echo off

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET Opt=%1%

pushd %BuildDir%

IF "%Opt%"=="assets" (
	call tool_build_assets.exe
	GOTO End
) 

IF "%Opt%"=="atlas" (
	call tool_build_atlas.exe
	GOTO End
) 

call sdl_platform.exe

:End

popd