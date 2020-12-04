@echo off

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET Opt=%1%

pushd %BuildDir%

IF "%Opt%"=="atlas" ( 
	call devenv tool_build_atlas.exe
	GOTO End
)

IF "%Opt%"=="assets" (
	call devenv tool_build_assets.exe
	GOTO End
)

call devenv platform_sdl.exe

:End 

popd
