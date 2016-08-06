@echo off

setx SDL2DIR "C:\Program Files\sdl2\SDL2-2.0.4"
setx SDL2IMAGEDIR "C:\Program Files\sdl2\SDL2_image-2.0.1"
setx SDL2TTFDIR "C:\Program Files\sdl2\SDL2_ttf-2.0.14"
setx LUA_DIR "C:\Program Files\lua-5.2.3"
setx VisualStudioVersion "14.0"

pushd build

cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

popd

msbuild ./build/becoming_portland.sln
