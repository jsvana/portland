@echo off

pushd build

cmake -G"Visual Studio 14" -DSFML_ROOT="C:\Users\jsvana\Documents\SFML-2.4.1" ..

popd

msbuild ./build/portland.vcxproj