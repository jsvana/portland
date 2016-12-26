#include "dialog.h"

namespace visual {

Dialog::Dialog(std::string message) : message_(message) {
  font_.loadFromFile("assets/fonts/arcade.ttf");
  map_ = std::make_unique<map::Map>("assets/maps/dialog.json");
  map_->setPosition(position_.x, position_.y);

  indicatorOffset_ = 0;
  lineIndex_ = 0;
  selectedChoice_ = -1;

  // Split the message into lines
  std::string tmp(message);
  while (tmp.length() > 0) {
    std::string line;
    if (tmp.length() > LINE_LENGTH) {
      // Split on last space
      auto idx = tmp.find_last_of(" ", LINE_LENGTH);
      if (idx == std::string::npos) {
        line = tmp.substr(0, LINE_LENGTH);
        tmp = tmp.substr(LINE_LENGTH, tmp.length());
      } else {
        line = tmp.substr(0, idx);
        // +1 to skip space
        tmp = tmp.substr(idx + 1, tmp.length());
      }
    } else {
      line = tmp;
      tmp = "";
    }
    sf::Text text;
    text.setFont(font_);
    text.setString(line);

    text.setColor(sf::Color::White);

    lines_.push_back(text);
  }

  reflowText();

  // Debounce input at the start
  selectFrames_ = FRAME_DEBOUNCE_DELAY;
}

void Dialog::addOptions(const std::vector<std::string>& choices) {
  for (auto& choice : choices) {
    choices_.push_back(choice);
  }

  selectedChoice_ = choices_.size() - 1;

  reflowText();
}

void Dialog::setPosition(int x, int y) {
  position_.x = x;
  position_.y = y;

  map_->setPosition(position_.x, position_.y);

  reflowText();
}

void Dialog::reflowText() {
  int y = position_.y + TEXT_PADDING;

  if (lines_.size() <= VISIBLE_LINES) {
    moreIndicator_.reset();
  } else {
    if (!moreIndicator_) {
      moreIndicator_ = std::make_unique<sf::Text>();
      moreIndicator_->setFont(font_);
    }
    auto dim = moreIndicator_->getGlobalBounds();
    moreIndicator_->setPosition(
        position_.x + pixelWidth() - dim.width - TEXT_PADDING,
        position_.y + pixelHeight() - dim.height + indicatorOffset_);
  }

  // Reposition all visible lines
  unsigned int i = 0;
  for (auto& line : lines_) {
    line.setPosition(TEXT_PADDING, y);
    y += line.getGlobalBounds().height;
    if (i == VISIBLE_LINES) {
      break;
    }
    i += 1;
  }

  // Position and show choices and choice indicator
  if (lines_.size() == VISIBLE_LINES - 1) {
    if (!choiceIndicator_) {
      choiceIndicator_ = std::make_unique<sf::Text>();
      choiceIndicator_->setFont(font_);
    }
    int offset = 0;
    int padding = 10;
    for (unsigned int i = 0; i < choices_.size(); i++) {
      sf::Text choiceText;
      choiceText.setFont(font_);
      choiceText.setString(choices_[i]);
      choiceText.setColor(sf::Color::White);
      auto dim = choiceText.getGlobalBounds();
      offset += dim.width + padding;
      if (i == (unsigned int)selectedChoice_) {
        choiceIndicator_->setPosition(position_.x + pixelWidth() - offset -
                                          dim.width - indicatorOffset_ + 2,
                                      position_.y + pixelHeight() - dim.height);
      }
      choiceText.setPosition(position_.x + pixelWidth() - offset,
                             position_.y + pixelHeight() - dim.height);
      choicesText_.push_back(choiceText);
    }
  }
}

void Dialog::handleEvent(sf::Event& event) {
  if (event.type == sf::Event::KeyPressed) {
    switch (event.key.code) {
      case sf::Keyboard::Left:
      case sf::Keyboard::A:
        if (choices_.empty()) {
          break;
        }
        selectedChoice_ = (selectedChoice_ - 1) % choices_.size();
        reflowText();
        break;
      case sf::Keyboard::Right:
      case sf::Keyboard::D:
        if (choices_.empty()) {
          break;
        }
        selectedChoice_ = (selectedChoice_ + 1) % choices_.size();
        reflowText();
        break;
      case sf::Keyboard::Return:
        lineIndex_ += 1;
        if (lines_.size() >= VISIBLE_LINES) {
          lines_.pop_front();
        } else {
          completed_ = true;
        }
        reflowText();
        break;
      default:
        break;
    }
  }
}

bool Dialog::update(sf::Time& time) {
  if (selectFrames_ > 0) {
    selectFrames_ -= 1;
  }

  time_ += time;
  if (time_ >= sf::milliseconds(500)) {
    indicatorOffset_ = (indicatorOffset_ + 2) % BOUNCE_DISTANCE;
    time_ = sf::seconds(0);

    reflowText();
  }

  return !completed_;
}

void Dialog::render(sf::RenderTarget& window) {
  map_->render(window, camera_);
  unsigned int i = 0;
  for (auto& line : lines_) {
    window.draw(line);
    i += 1;
    if (i == VISIBLE_LINES) {
      break;
    }
  }
  for (auto& choiceText : choicesText_) {
    window.draw(choiceText);
  }
  if (moreIndicator_) {
    window.draw(*moreIndicator_);
  }
  if (choiceIndicator_) {
    window.draw(*choiceIndicator_);
  }
}

namespace DialogManager {

std::deque<std::unique_ptr<Dialog>> dialogs;
std::deque<Id> uids;
std::unordered_map<Id, int> choices;
std::unique_ptr<Dialog> closedDialog_;
Id nextUid = 0;

Id queueDialog(Dialog* dialog) {
  nextUid += 1;
  dialogs.push_back(std::unique_ptr<Dialog>(dialog));
  uids.push_back(nextUid);
  return nextUid;
}

Dialog* getDialogByUid(const Id uid) {
  for (unsigned int i = 0; i < dialogs.size(); i++) {
    if (uids[i] == uid) {
      return dialogs[i].get();
      ;
    }
  }
  return nullptr;
}

bool addDialogOption(const Id uid, std::string option) {
  const auto& dialog = getDialogByUid(uid);
  if (!dialog) {
    return false;
  }
  dialog->addOptions({option});
  return true;
}

bool setDialogCallback(const Id uid, visual::DialogCallback callback) {
  const auto& dialog = getDialogByUid(uid);
  if (!dialog) {
    return false;
  }
  dialog->callbackFunc = callback;
  return true;
}

const std::unique_ptr<Dialog>& closedDialog() { return closedDialog_; }

void clearClosedDialog() { closedDialog_.release(); }

int dialogChoice(const Id uid) { return choices[uid]; }

void handleEvent(sf::Event& event) {
  if (dialogs.empty()) {
    return;
  }
  dialogs.front()->handleEvent(event);
}

bool update(sf::Time& time) {
  if (dialogs.empty()) {
    return false;
  }

  dialogs.front();

  if (dialogs.front()->update(time)) {
    return true;
  } else {
    int uid = uids.front();

    closedDialog_ = std::move(dialogs.front());

    if (closedDialog_->hasChoices()) {
      choices[uid] = closedDialog_->getChoice();
    } else {
      choices[uid] = -1;
    }

    uids.pop_front();
    dialogs.pop_front();
  }

  return false;
}

void render(sf::RenderTarget& window) {
  if (dialogs.empty()) {
    return;
  }
  dialogs.front()->render(window);
}
}  // namespace DialogManager

}  // namespace visual
