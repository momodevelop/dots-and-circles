@echo off 
IF "%1"=="" (SET Folder=test) ELSE (SET Folder=%1)

SET RootDir=%me%..
SET SandboxDir=%RootDir%\sandbox\%Folder%

pushd %SandboxDir%

call test.exe

:End

popd
