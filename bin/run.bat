@echo off

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET Opt=%1%

pushd %BuildDir%

IF "%Opt%"=="assets" (
	call tool_build_assets.exe
) ELSE (
	call sdl_platform.exe
)
popd