@echo off

if "%1" == "" (
	echo "Usage: %0 <sfml_root>"
	exit /b 1
)

if not exist "%1" (
	echo "Can't find SFML_ROOT %1"
	exit /b 1
)

set SFML_ROOT="%1"

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

.\build\Debug\portland.exe

:CopyDll
set dll_path="%SFML_ROOT%\bin\%~1"
set out_path=".\build\Debug"
if not exist "%out_path%\%~1" (
	xcopy /i "%SFML_ROOT%\bin\%~1" "%out_path%" > NUL
)
exit /b 0
