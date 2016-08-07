#pragma once

#include "../util.h"

#include <SFML/Graphics.hpp>

#include <queue>
#include <string>

namespace visual {

  namespace Console {
    const unsigned int MAX_SIZE = 3;

    void runCommand(std::string command);

    void initialize();

    void show();

    void hide();

    bool visible();

    void handleEvent(sf::Event &event);

    /**
     * Handles key input and text reflow
     *
     * @param time Amount of time since last update
     * @return Whether dialog is still open or not
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
