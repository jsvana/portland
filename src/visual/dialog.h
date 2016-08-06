#pragma once

#include "../map.h"
#include "../util.h"
#include "text.h"

#include <SFML/Graphics.hpp>

#include <deque>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

namespace visual {

  /**
   * Class to load, render, and handle input for in-game dialogs
   */
  class Dialog {
   private:
    const int FRAME_DEBOUNCE_DELAY = 5;

    const int TEXT_PADDING = 10;

    // TODO(jsvana): make these dynamic based on size of dialog
    const unsigned int LINE_LENGTH = 31;
    const unsigned int VISIBLE_LINES = 3;

    // Indicator animation constants
    const unsigned int FRAME_TICKS_INTERVAL = 24;
    const int BOUNCE_DISTANCE = 4;

    // Kind of a hack. Contains the background of the dialog
    std::unique_ptr<Map> map_;

    // Dialog message to display
    std::string message_;

    // Choices in dialog
    std::vector<std::string> choices_;

    std::shared_ptr<sf::Font> font_;

    // Vector of renderable lines of text
    std::deque<sf::Text> lines_;
    // Vector of renderable choices for dialog with options (e.g., "Yes" and
    // "No")
    std::vector<sf::Text> choicesText_;

    int selectedChoice_;

    unsigned int lastTicks_;
    int indicatorOffset_;

    // Renderable textures for dialog indicators
    std::shared_ptr<sf::Text> moreIndicator_;
    std::shared_ptr<sf::Text> choiceIndicator_;

    int lineIndex_;

    // Position of dialog (almost always the bottom of the screen)
    Point position_;

    // Always at (0, 0)
    Point camera_;

    // Used for debouncing input
    unsigned int selectFrames_;

    /**
     * Changes dialog text position, shows and hides indicators, and shows
     * options
     * as state of the dialog changes
     */
    void reflowText();

   public:
    // Name of Lua function to call after dialog is complete
    std::string callbackFunc;

    Dialog(std::string message);

    /**
     * Add options to the dialog
     *
     * Will reflow text
     *
     * @param choices Vector of choices to add
     */
    void addOptions(const std::vector<std::string> &choices);

    /**
     * Determines whether dialog has choices
     *
     * @return Whether dialog has choices
     */
    bool hasChoices() { return !choices_.empty(); }

    /**
     * Gets the index in the choices vector of the selected choice
     *
     * @return Index of selected choice
     */
    int getChoice() { return selectedChoice_; }

    /**
     * Sets dialog position
     *
     * @param x New x coordinate of dialog
     * @param y New y coordinate of dialog
     */
    void setPosition(int x, int y);

    /**
     * Sets dialog position with a Point
     *
     * @param position Point of new dialog position
     */
    void setPosition(Point position);

    /**
     * Gets dialog width in pixels
     *
     * @return Dialog width in pixels
     */
    int pixelWidth() { return map_->pixelWidth(); }

    /**
     * Gets dialog height in pixels
     *
     * @return Dialog height in pixels
     */
    int pixelHeight() { return map_->pixelHeight(); }

    /**
     * Sets dialog text color
     *
     * @param color New dialog text color
     */
    // TODO(jsvana): Actually set all the colors
    // and the default color here
    void setColor(sf::Color color) {
      for (auto &line : lines_) {
        line.setColor(color);
      }
    }

    /**
     * Handles key input and text reflow
     *
     * @param ticks Number of ticks since start
     * @return Whether dialog is still open or not
     */
    bool update(unsigned long ticks);

    /**
     * Renders the dialog
     *
     * @param window Window to render to
     */
    void render(sf::RenderTarget &window);
  };

  namespace DialogManager {

    /**
     * Queues a new dialog to be shown.
     *
     * @param dialog Dialog to show
     * @return UID of dialog for fetching results and getting status
     */
    unsigned int queueDialog(Dialog *dialog);

    /**
     * Gets a specific dialog object by its UID
     *
     * @param uid UID of dialog
     * @return Found dialog object or nullptr if not found
     */
    std::shared_ptr<Dialog> getDialogByUid(unsigned int uid);

    /**
     * Wrapper to add a dialog option by UID
     *
     * @param uid UID of dialog to add option to
     * @param option Option to add
     * @return Whether operation was successful
     */
    bool addDialogOption(unsigned int uid, std::string option);

    /**
     * Wrapper to set a dialog's callback function by UID
     *
     * @param uid UID of dialog to add option to
     * @param callback Name of callback function
     * @return Whether operation was successful
     */
    bool setDialogCallback(unsigned int uid, std::string callback);

    /**
     * Gets the most recently closed dialog or nullptr if there isn't one
     * or the closed dialog has been cleared
     *
     * @return Most recently closed dialog
     */
    std::shared_ptr<Dialog> closedDialog();

    /**
     * Clears the most recently closed dialog
     */
    void clearClosedDialog();

    /**
     * Wrapper to get a dialog's choice by UID
     *
     * @param uid UID of dialog to get choice from
     * @return Dialog's choice
     */
    int dialogChoice(unsigned int uid);

    /**
     * Update the first dialog in the queue
     *
     * @param ticks Number of ticks since start
     * @return Whether dialogs are cleared
     */
    bool update(unsigned int ticks);

    /**
     * Renders the first dialog in the queue
     *
     * @param window Window to render to
     */
    void render(sf::RenderTarget &window);
  }

}  // namespace visual
