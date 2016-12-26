#pragma once

#include "../map.h"
#include "../util.h"

#include <SFML/Graphics.hpp>

#include <deque>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

namespace visual {

typedef std::function<void(int)> DialogCallback;

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
  const util::Tick FRAME_TICKS_INTERVAL = 24;
  const int BOUNCE_DISTANCE = 4;

  // Kind of a hack. Contains the background of the dialog
  std::unique_ptr<map::Map> map_;

  // Dialog message to display
  std::string message_;

  // Choices in dialog
  std::vector<std::string> choices_;

  sf::Font font_;

  // Vector of renderable lines of text
  std::deque<sf::Text> lines_;
  // Vector of renderable choices for dialog with options (e.g., "Yes" and
  // "No")
  std::vector<sf::Text> choicesText_;

  int selectedChoice_;

  bool completed_ = false;

  sf::Time time_;
  int indicatorOffset_;

  // Renderable textures for dialog indicators
  std::unique_ptr<sf::Text> moreIndicator_;
  std::unique_ptr<sf::Text> choiceIndicator_;

  int lineIndex_;

  // Position of dialog (almost always the bottom of the screen)
  sf::Vector2f position_;

  // Always at (0, 0)
  sf::Vector2f camera_;

  // Used for debouncing input
  unsigned int selectFrames_;

  /**
   * Changes dialog text position, shows and hides indicators, and shows
   * options
   * as state of the dialog changes
   */
  void reflowText();

 public:
  // ChaiScript function to call after dialog is complete
  DialogCallback callbackFunc;

  Dialog(std::string message);

  /**
   * Add options to the dialog
   *
   * Will reflow text
   *
   * @param choices Vector of choices to add
   */
  void addOptions(const std::vector<std::string>& choices);

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
   * Sets dialog position with a sf::Vector2f
   *
   * @param position Point of new dialog position
   */
  void setPosition(sf::Vector2f position);

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
    for (auto& line : lines_) {
      line.setColor(color);
    }
  }

  void handleEvent(sf::Event& event);

  /**
   * Handles key input and text reflow
   *
   * @param time Amount of time since last update
   * @return Whether dialog is still open or not
   */
  bool update(sf::Time& time);

  /**
   * Renders the dialog
   *
   * @param window Window to render to
   */
  void render(sf::RenderTarget& window);
};

namespace DialogManager {

typedef unsigned int Id;

/**
 * Queues a new dialog to be shown.
 *
 * @param dialog Dialog to show
 * @return UID of dialog for fetching results and getting status
 */
Id queueDialog(Dialog* dialog);

/**
 * Gets a specific dialog object by its UID
 *
 * @param uid UID of dialog
 * @return Found dialog object or nullptr if not found
 */
Dialog* getDialogByUid(const Id uid);

/**
 * Wrapper to add a dialog option by UID
 *
 * @param uid UID of dialog to add option to
 * @param option Option to add
 * @return Whether operation was successful
 */
bool addDialogOption(const Id uid, std::string option);

/**
 * Wrapper to set a dialog's callback function by UID
 *
 * @param uid UID of dialog to add option to
 * @param callback Callback function
 * @return Whether operation was successful
 */
bool setDialogCallback(const Id uid, visual::DialogCallback callback);

/**
 * Gets the most recently closed dialog or nullptr if there isn't one
 * or the closed dialog has been cleared
 *
 * @return Most recently closed dialog
 */
const std::unique_ptr<Dialog>& closedDialog();

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
int dialogChoice(const Id uid);

void handleEvent(sf::Event& event);

/**
 * Update the first dialog in the queue
 *
 * @param time Amount of time since last update
 * @return Whether dialogs are cleared
 */
bool update(sf::Time& time);

/**
 * Renders the first dialog in the queue
 *
 * @param window Window to render to
 */
void render(sf::RenderTarget& window);

}  // namespace DialogManager

}  // namespace visual
