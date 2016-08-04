#pragma once

#include <SDL_image.h>
#include <SDL_ttf.h>

#include <string>

namespace AssetManager {

/**
 * Builds and returns a font to be used in text rendering
 *
 * @param path Path to typeface to use
 * @param size Size of font to render
 * @return Generated font
 */
TTF_Font *getFont(const std::string &path, int size);

/**
 * Builds an SDL texture object from the given image path
 *
 * @param path Path to image to load
 * @return Loaded texture
 */
SDL_Texture *getTexture(const std::string &path);

/**
 * Cleans up all loaded assets
 */
void destroy();
}
