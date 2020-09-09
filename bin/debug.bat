@echo off

SET RootDir=%me%..
SET BuildDir=%RootDir%\build

pushd %BuildDir%

IF "%Opt%"=="atlas" ( 
	call devenv tool_build_atlas.exe
	GOTO End
)

IF "%Opt%"=="assets" (
	call devenv tool_build_assets.exe
	GOTO End
)

call devenv sdl_platform.exe

:End 

popd