#pragma once

#include "../util.h"

#include <SFML/Graphics.hpp>

#include <queue>
#include <string>

namespace visual {

  namespace Console {
    const unsigned int MAX_SIZE = 3;

    /**
     * Runs the given command string against Lua and returns
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
    void handleEvent(sf::Event &event);

    /**
     * Animates the cursor
     *
     * @param time Amount of time since last update
     * @return Always returns true
     */
    bool update(sf::Time &time);

    /**
     * Renders the console
     *
     * @param window Window to render to
     */
    void render(sf::RenderTarget &window);

  } // namespace Console

}  // namespace visual
