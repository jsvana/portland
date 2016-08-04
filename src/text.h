#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>

/**
 * Class to load and render text on screen
 */
class Text {
  std::string text_;

  int size_;
  SDL_Texture *texture_;
  SDL_Color color_;
  SDL_Rect position_;
  TTF_Font *font_;
  bool visible_;

  /**
   * Creates the font texture
   */
  void load();

 public:
  Text(const std::string &text, int size);
  ~Text() { SDL_DestroyTexture(texture_); }

  /**
   * Sets the color for the text object
   *
   * @param color Color to change to
   */
  void setColor(SDL_Color color) {
    color_ = color;
    load();
  }

  /**
   * Gets the width of the text object
   *
   * @return Width of the text object
   */
  int width() { return position_.w; }

  /**
   * Gets the height of the text object
   *
   * @return Height of the text object
   */
  int height() { return position_.h; }

  /**
   * Makes text object visible
   */
  void show() { visible_ = true; }

  /**
   * Makes text object invisible
   */
  void hide() { visible_ = false; }

  /**
   * Gets visibility of text object
   *
   * @return Whether the text object is visible
   */
  bool visible() { return visible_; }

  /**
   * Sets position of the text object from the top left corner
   *
   * @param x New x coordinate of text object
   * @param y New y coordinate of text object
   */
  void setPosition(int x, int y) { position_.x = x, position_.y = y; }

  /**
   * Sets position of the text object from the center
   *
   * @param x New x coordinate of the center of the text object
   * @param y New y coordinate of the center of the text object
   */
  void setPositionCenter(int x, int y);

  /**
   * Renders the text object. Will do nothing if the object is
   * invisible.
   */
  void render();
};
