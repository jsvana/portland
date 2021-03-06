#pragma once

#include "../state.h"
#include "../util.h"

#include <SFML/Graphics.hpp>
#include <chaiscript/chaiscript.hpp>

#include <string>

namespace visual {

namespace Console {

const std::size_t MAX_HISTORY = 3;
const float MARGIN = 2;
const float PADDING = 2;
const int FONT_SIZE = 8;

/**
 * Gets the specified value from ChaiScript
 *
 * @template T Type to fetch
 * @param value Name of value to fetch
 * @return Fetched value
 */
template <typename T>
T getValue(std::string value) {
  return static_cast<T>(GameState::chai().eval<T>(value));
}

/**
 * Runs the given command string against ChaiScript and returns
 * the result
 *
 * @param command Command to run
 * @return Output of command
 */
std::string runCommand(std::string command);

/**
 * Initializes all console assets
 */
void initialize();

/**
 * Shows the console
 */
void show();

/**
 * Hides the console
 */
void hide();

/**
 * Gets whether or not console is visible
 *
 * @return Whether or not console is visible
 */
bool visible();

/**
 * Handles key input
 *
 * @param event Event to handle
 */
void handleEvent(sf::Event& event);

/**
 * Animates the cursor
 *
 * @param time Amount of time since last update
 * @return Always returns true
 */
bool update(sf::Time& time);

/**
 * Renders the console
 *
 * @param window Window to render to
 */
void render(sf::RenderTarget& window);

}  // namespace Console

}  // namespace visual
