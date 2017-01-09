# Becoming Portland

A basic sidescrolling platformer (metroidvania).

## Scripting

This game has a ChaiScript API. You can see how it works by looking in [assets/scripts/game.chai](assets/scripts/game.chai).

### Quick Start

There are two defined methods that will be called in the game's script: `init()` and `update()` (neither take arguments). In each of these you can interact with the world and add event callbacks.

For interactions, there are a few functions:

```c++
// Loads a new map and pushes it on the map stack
loadMap(string path);

// Loads a character and moves the camera to the character's position.
loadCharacter(string path, int x, int y, int tile);

// Creates an event to trigger on hitting the tile.
registerTileEvent(int x, int y, func callback);

// Get and set global game flags.
getFlag(string flag);
setFlag(string flag, bool value);

// Get and set global game values.
getValue(string key);
setValue(string key, int value);

// Add callbacks to fire when flags and values change.
addFlagChangeCallback(string flag, func callback);
addValueChangeCallback(string key, func callback);

// Get various game sprites. Returned objects should support all expected methods and properties.
getHero();
getSprite(int id);
getNpc(int id);
getItem(int id);
```

### ChaiScript Console

The game also supports mutating game state from an in-game console. To use the console, open it with Ctrl-C.

To run a ChaiScript statement:

    > run global x = "foo";

To get a top-level ChaiScript value as a string:

    > get x
    -> foo

To get an integer or boolean you must use:

    > run x = 5;
    > geti x
    -> 5

    > run x = true;
    > getb x
    -> 5

## Dependencies

Build configuration uses CMake.

Packages: `sfml`

## Build and run

### Linux/Mac

```
$ mkdir build
$ cd build && cmake .. && cd ..
$ ./scripts/run
```

### Windows

Download [SFML](http://www.sfml-dev.org/download.php) and make sure you have MSBuild.

```
$ mkdir build
$ ./scripts/run.bat <path\to\extracted\sfml\root>
```

## Credits

Thanks to:

- [@lefticus](https://github.com/lefticus) for [ChaiScript](https://github.com/ChaiScript/ChaiScript)

- [@nlohmann](https://github.com/nlohmann) for [json](https://github.com/nlohmann/json)

- [The Chayed - KIIRA](http://opengameart.org/users/the-chayed-kiira) for the tileset

- Alex Arens for the awesome hipster sprites

## License

[MIT](LICENSE)
