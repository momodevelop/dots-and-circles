@echo off

SET RootDir=%me%..
SET BuildDir=%RootDir%\build
SET Opt=%1%

pushd %BuildDir%

IF "%Opt%"=="assets" (
	call devenv tool_build_assets.exe
	GOTO End
)

IF "%Opt%"=="win32" (
    call devenv platform_win32_opengl.exe
    GOTO End
)
call devenv platform_sdl_opengl.exe

:End 

popd
