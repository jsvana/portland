#include "dialog.h"

extern SDL_Renderer *renderer;

namespace visual {

  Dialog::Dialog(std::string message) : message_(message) {
    map_ = new Map("assets/maps/dialog.json");
    map_->setPosition(position_.x, position_.y);

    // Default color is white
    DEFAULT_COLOR.r = 255;
    DEFAULT_COLOR.g = 255;
    DEFAULT_COLOR.b = 255;

    moreIndicator_ = new Text("v", 15);
    moreIndicator_->setColor(DEFAULT_COLOR);
    moreIndicator_->hide();
    indicatorOffset_ = 0;

    lineIndex_ = 0;

    choiceIndicator_ = new Text(">", 15);
    choiceIndicator_->setColor(DEFAULT_COLOR);
    choiceIndicator_->hide();
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
      Text *text = new Text(line, 15);
      text->setColor(DEFAULT_COLOR);

      if (lines_.size() >= VISIBLE_LINES) {
        text->hide();
      }

      lines_.push_back(text);
    }

    reflowText();

    // Debounce input at the start
    selectFrames_ = FRAME_DEBOUNCE_DELAY;
  }

  void Dialog::addOptions(const std::vector<std::string> &choices) {
    for (auto &choice : choices) {
      choices_.push_back(choice);
      Text *choiceText = new Text(choice, 15);
      choiceText->setColor(DEFAULT_COLOR);
      choiceText->hide();
      choicesText_.push_back(choiceText);
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

    // Hide all lines before the first visible line
    for (int i = 0; i < lineIndex_; i++) {
      lines_[i]->hide();
    }

    // Show all lines that should be visible
    unsigned int upperBound =
        std::min<int>(lines_.size() - lineIndex_, lineIndex_ + VISIBLE_LINES);
    for (unsigned int i = lineIndex_; i < upperBound; i++) {
      lines_[i]->show();
    }

    // Determine whether or not more indicator should be shown
    moreIndicator_->setPosition(
        position_.x + pixelWidth() - moreIndicator_->width() - TEXT_PADDING,
        position_.y + pixelHeight() - moreIndicator_->height() +
            indicatorOffset_);
    if (lineIndex_ + VISIBLE_LINES >= lines_.size()) {
      moreIndicator_->hide();
    } else {
      moreIndicator_->show();
    }

    // Reposition all visible lines
    for (auto &line : lines_) {
      if (line->visible()) {
        line->setPosition(TEXT_PADDING, y);
        y += line->height();
      }
    }

    // Position and show choices and choice indicator
    if (lines_.back()->visible()) {
      int offset = 0;
      int padding = 10;
      for (unsigned int i = 0; i < choices_.size(); i++) {
        Text *choiceText = choicesText_[i];
        offset += choiceText->width() + padding;
        if (i == (unsigned int)selectedChoice_) {
          choiceIndicator_->setPosition(
              position_.x + pixelWidth() - offset - choiceIndicator_->width() -
                  indicatorOffset_ + 2,
              position_.y + pixelHeight() - choiceText->height());
          choiceIndicator_->show();
        }
        choiceText->setPosition(
            position_.x + pixelWidth() - offset,
            position_.y + pixelHeight() - choiceText->height());
        choiceText->show();
      }
    }
  }

  bool Dialog::update(unsigned long ticks) {
    if (selectFrames_ > 0) {
      selectFrames_ -= 1;
    }

    if (ticks - lastTicks_ >= FRAME_TICKS_INTERVAL) {
      indicatorOffset_ = (indicatorOffset_ + 2) % BOUNCE_DISTANCE;
      lastTicks_ = ticks;

      reflowText();
    }

    auto state = SDL_GetKeyboardState(nullptr);
    if (state[SDL_SCANCODE_RETURN] && selectFrames_ == 0) {
      selectFrames_ = FRAME_DEBOUNCE_DELAY;

      lineIndex_ += 1;
      if (lineIndex_ + VISIBLE_LINES > lines_.size()) {
        return false;
      }
      reflowText();
    }

    if (!choices_.empty()) {
      if (state[SDL_SCANCODE_LEFT] && selectFrames_ == 0) {
        selectFrames_ = FRAME_DEBOUNCE_DELAY;
        selectedChoice_ = (selectedChoice_ - 1) % choices_.size();
        reflowText();
      }
      if (state[SDL_SCANCODE_RIGHT] && selectFrames_ == 0) {
        selectFrames_ = FRAME_DEBOUNCE_DELAY;
        selectedChoice_ = (selectedChoice_ + 1) % choices_.size();
        reflowText();
      }
    }

    return true;
  }

  void Dialog::render() {
    map_->render(camera_);
    for (const auto &line : lines_) {
      line->render();
    }
    for (auto &choiceText : choicesText_) {
      choiceText->render();
    }
    moreIndicator_->render();
    choiceIndicator_->render();
  }

  namespace DialogManager {

    std::deque<std::shared_ptr<Dialog>> dialogs;
    std::deque<unsigned int> uids;
    std::unordered_map<unsigned int, int> choices;
    std::shared_ptr<Dialog> closedDialog_ = nullptr;
    unsigned int nextUid = 0;

    unsigned int queueDialog(Dialog *dialog) {
      nextUid += 1;
      dialogs.push_back(std::shared_ptr<Dialog>(dialog));
      uids.push_back(nextUid);
      return nextUid;
    }

    std::shared_ptr<Dialog> getDialogByUid(unsigned int uid) {
      for (unsigned int i = 0; i < dialogs.size(); i++) {
        if (uids[i] == uid) {
          return dialogs[i];
          ;
        }
      }
      return nullptr;
    }

    bool addDialogOption(unsigned int uid, std::string option) {
      auto dialog = getDialogByUid(uid);
      if (dialog == nullptr) {
        return false;
      }
      dialog->addOptions({option});
      return true;
    }

    bool setDialogCallback(unsigned int uid, std::string callback) {
      auto dialog = getDialogByUid(uid);
      if (dialog == nullptr) {
        return false;
      }
      dialog->callbackFunc = callback;
      return true;
    }

    std::shared_ptr<Dialog> closedDialog() { return closedDialog_; }

    void clearClosedDialog() { closedDialog_ = nullptr; }

    int dialogChoice(unsigned int uid) { return choices[uid]; }

    bool update(unsigned int ticks) {
      if (dialogs.empty()) {
        return false;
      }

      auto dialog = dialogs.front();

      if (dialog->update(ticks)) {
        return true;
      } else {
        int uid = uids.front();

        closedDialog_ = dialog;

        if (dialog->hasChoices()) {
          choices[uid] = dialog->getChoice();
        } else {
          choices[uid] = -1;
        }

        uids.pop_front();
        dialogs.pop_front();
      }

      return false;
    }

    void render() {
      if (dialogs.empty()) {
        return;
      }
      dialogs.front()->render();
    }
  }  // namespace DialogManager

}  // namespace visual
