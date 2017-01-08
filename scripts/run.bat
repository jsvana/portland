@echo off

set SFML_ROOT="C:\Users\jsvana\Documents\SFML-2.4.1"

pushd build

cmake -G"Visual Studio 14" -DSFML_ROOT=%SFML_ROOT% ..

popd

msbuild .\build\portland.vcxproj

if not exist .\build\Debug\assets (
	xcopy /s /i  .\assets .\build\Debug\assets > NUL
)

call :CopyDll "sfml-graphics-d-2.dll"
call :CopyDll "sfml-system-d-2.dll"
call :CopyDll "sfml-window-d-2.dll"

:CopyDll
set dll_path="%SFML_ROOT%\bin\%~1"
set out_path=".\build\Debug"
if not exist "%out_path%\%~1" (
	xcopy /i "%SFML_ROOT%\bin\%~1" "%out_path%" > NUL
)
EXIT /B 0