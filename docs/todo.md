# TODO

- [x] Proper multiupdate game loop

- [x] Interpolated rendering in game loop

- [x] Scrolling

- [x] Proper commented asset file formats (JSON?)

- [x] Text

- [ ] Sound

- [x] Walkability

- [x] Interactive entities

  - [x] NPCs that talk

- [x] Dialogs with options

- [ ] Combat

  - [ ] Entity health

- [ ] Items

- [x] Move Lua API to GameState

- [x] Ladders

- [x] Better scrolling

- [ ] Doors

- [ ] Interactive tiles on button press

- [ ] Serialization of GameState, saving game

- [ ] Only render visible parts of map

## Code Quality

- [ ] Standardize method decorators (const, noexcept) and argument passing

- [ ] std::unique_ptr and friends

## Bugs

- [x] Allow rendering of tile when its id is 0

- [x] Figure out why Map tileset entities aren't walkable

- [x] Remove dependence on renderer (make global?)

- [ ] Fix dialog segfault when leaving town

- [x] Don't allow restart of jump when character hits something above

- [x] Snap character to tile below when falling to prevent fall jitter

- [ ] Fix jump jitter when jumping to edge of map (possibly unnecessary)

- [ ] Handle window killing
