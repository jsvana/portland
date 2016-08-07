# Becoming Portland

A basic sidescrolling platformer (metroidvania).

## Scripting

This game has a Lua API. You can see how it works by looking in [assets/scripts/game.lua].

    TODO(jsvana): fill out Lua API docs

### Lua Console

The game also supports mutating game state from an in-game console. To use the console, open it with Ctrl-C.

To run a Lua statement:

    > run x = "foo"

To get a top-level Lua value as a string:

    > get x
    -> foo

To get an integer or boolean you must use:

    > run x = 5
    > geti x
    -> 5

    > run x = true
    > getb x
    -> 5

These are necessary because we cannot peak at the type of the Lua value in C++.

## Dependencies

### OSX

Xcode (Tested with 6.1.1)

`brew install sdl2 sdl2_ttf sdl2_image cmake`

### Linux

Packages: `cmake`, `sdl2`, `sdl2_gfx`, `sdl2_image`, `sdl2_mixer`, `sdl2_ttf`, `smpeg2`

## Build and run

# Linux/Mac

    $ ./scripts/run

# Windows
Ensure that run.bat is correctly pointing to all required libraries

    $ ./scripts/run.bat

## Credits

Borrowed from [Will Ushers SDL2 Tutorials](http://www.willusher.io/sdl2%20tutorials/2014/03/06/lesson-0-cmake/)

Thanks to @nlohmann for [json](https://github.com/nlohmann/json)

Thanks to @jeremyong for [Selene](https://github.com/jeremyong/Selene)

Thanks to [The Chayed - KIIRA](http://opengameart.org/users/the-chayed-kiira) for the tileset
