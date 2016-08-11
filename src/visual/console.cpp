#include "console.h"

#include "../constants.h"

#include <glog/logging.h>

#include <deque>

namespace visual {

  namespace Console {
    // TODO(jsvana): base these off of text height
    const float INPUT_HEIGHT = 40;
    const float TOTAL_HEIGHT = 70;

    bool visible_ = false;

    bool cursor_ = true;

    sf::Time time_;

    std::deque<std::string> commands_;
    std::deque<sf::Text> outputs_;

    sf::FloatRect dimensions_;

    sf::Font font_;

    sf::Text prompt_;
    sf::Text command_;

    int selectedCommand_ = -1;

    std::string runCommand(std::string command) {
      if (command == "exit") {
        hide();
        return "Done";
      } else if (command.find("run ") == 0) {
        command = command.substr(4);
        LOG(INFO) << "Running Lua string: " << command;
        //GameState::lua()(command.c_str());
        return "Done";
      } else if (command.find("get") == 0) {
        command = command.substr(3);
        if (command.length() == 0) {
          return "Malformed command";
        }
        std::string val;
        if (command.find("s ") == 0 || command.find(" ") == 0) {
          val = getValue<std::string>(command.substr(2));
        } else if (command.find("b ") == 0) {
          //bool x = getValue<bool>(command.substr(2));
          bool x = true;
          if (x) {
            val = "true";
          } else {
            val = "false";
          }
        } else if (command.find("i ") == 0) {
          //val = std::to_string(getValue<int>(command.substr(2)));
          val = 1;
        }
        return "-> " + val;
      } else {
        return "Malformed command";
      }
    }

    void initialize() {
      font_.loadFromFile("assets/fonts/Anonymous.ttf");

      prompt_.setFont(font_);
      prompt_.setCharacterSize(FONT_SIZE);
      prompt_.setPosition(MARGIN, MARGIN);
      prompt_.setString(">");
      prompt_.setColor(sf::Color::White);

      auto size = prompt_.getGlobalBounds();
      command_.setFont(font_);
      command_.setCharacterSize(FONT_SIZE);
      command_.setPosition(MARGIN + 2 * size.width, MARGIN);
      command_.setColor(sf::Color::White);
    }

    void show() { visible_ = true; }

    void hide() { visible_ = false; }

    bool visible() { return visible_; }

    void handleEvent(sf::Event& event) {
      if (event.type == sf::Event::TextEntered) {
        if (event.text.unicode > 31 && event.text.unicode < 127) {
          command_.setString(command_.getString() +
                             static_cast<char>(event.text.unicode));
        }
      } else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::C) {
          if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            hide();
          }
        } else if (event.key.code == sf::Keyboard::BackSpace) {
          auto str = command_.getString().toAnsiString();
          if (str.length() > 0) {
            str.pop_back();
            command_.setString(str);
          }
        } else if (event.key.code == sf::Keyboard::Return) {
          auto str = command_.getString().toAnsiString();
          if (str == "") {
            return;
          }
          commands_.push_front(str);

          auto output = runCommand(str);

          sf::Text line;
          line.setFont(font_);
          line.setCharacterSize(FONT_SIZE);
          line.setColor(sf::Color::White);
          line.setString(output);
          outputs_.push_front(line);

          selectedCommand_ = -1;

          command_.setString("");
        } else if (event.key.code == sf::Keyboard::Up) {
          if (selectedCommand_ < (int)commands_.size() - 1) {
            selectedCommand_ += 1;
            command_.setString(commands_[selectedCommand_]);
          }
        } else if (event.key.code == sf::Keyboard::Down) {
          if (selectedCommand_ >= 0) {
            selectedCommand_ -= 1;
            if (selectedCommand_ == -1) {
              command_.setString("");
            } else {
              command_.setString(commands_[selectedCommand_]);
            }
          }
        }
      }
    }

    bool update(sf::Time& time) {
      time_ += time;
      if (time_ >= sf::milliseconds(500)) {
        cursor_ = !cursor_;
        time_ = sf::seconds(0);
      }
      return true;
    }

    void render(sf::RenderTarget& window) {
      auto promptSize = prompt_.getGlobalBounds();

      sf::RectangleShape background;
      background.setPosition(0, 0);
      background.setSize(sf::Vector2f(
          SCREEN_WIDTH,
          (2 * promptSize.height + 2 * PADDING) * (MAX_HISTORY + 1) +
              2 * MARGIN));
      background.setFillColor(sf::Color(42, 42, 42, 142));
      window.draw(background);

      window.draw(prompt_);
      window.draw(command_);

      auto size = command_.getGlobalBounds();

      if (cursor_) {
        sf::RectangleShape cursor;
        cursor.setPosition(size.left + size.width + 1, MARGIN);
        cursor.setSize(sf::Vector2f(promptSize.width, 2 * promptSize.height));
        cursor.setFillColor(sf::Color::White);
        window.draw(cursor);
      }

      float y = MARGIN + 2 * promptSize.height + PADDING;
      for (unsigned int i = 0; i < outputs_.size(); i++) {
        outputs_[i].setPosition(MARGIN, y);
        window.draw(outputs_[i]);
        y += 2 * outputs_[i].getGlobalBounds().height + PADDING;
        if (i + 1 == MAX_HISTORY) {
          break;
        }
      }
    }
  }
}
