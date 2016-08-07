#include "console.h"

#include "../constants.h"
#include "../state.h"

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
      if (command.find("run ") == 0) {
        command = command.substr(4);
        util::out()->info("Running Lua string: \"{}\"", command);
        GameState::lua()(command.c_str());
        return "Done";
      } else if (command.find("get ") == 0) {
        command = command.substr(4);
        return "-> " +
               static_cast<std::string>(GameState::lua()[command.c_str()]);
      } else {
        return "Malformed command";
      }
    }

    void initialize() {
      font_.loadFromFile("assets/fonts/arcade.ttf");

      prompt_.setFont(font_);
      prompt_.setCharacterSize(15);
      prompt_.setPosition(2, 0);
      prompt_.setString(">");
      prompt_.setColor(sf::Color::White);

      command_.setFont(font_);
      command_.setCharacterSize(15);
      command_.setPosition(10, 0);
      command_.setColor(sf::Color::White);
    }

    void show() { visible_ = true; }

    void hide() { visible_ = false; }

    bool visible() { return visible_; }

    void handleEvent(sf::Event &event) {
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
          line.setCharacterSize(15);
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

    bool update(sf::Time &time) {
      time_ += time;
      if (time_ >= sf::milliseconds(500)) {
        cursor_ = !cursor_;
        time_ = sf::seconds(0);
      }
      return true;
    }

    void render(sf::RenderTarget &window) {
      sf::RectangleShape background;
      background.setPosition(0, 0);
      background.setSize(sf::Vector2f(SCREEN_WIDTH, TOTAL_HEIGHT));
      background.setFillColor(sf::Color(42, 42, 42, 42));
      window.draw(background);

      background.setPosition(dimensions_.left, dimensions_.top);
      background.setSize(sf::Vector2f(dimensions_.width, INPUT_HEIGHT));
      background.setFillColor(sf::Color(150, 150, 150, 255));
      window.draw(background);

      window.draw(prompt_);
      window.draw(command_);

      auto size = command_.getGlobalBounds();

      if (cursor_) {
        sf::RectangleShape cursor;
        cursor.setPosition(size.width + 14, 2);
        cursor.setSize(sf::Vector2f(5, 10));
        cursor.setFillColor(sf::Color::White);
        window.draw(cursor);
      }

      float y = 15;
      for (unsigned int i = 0; i < outputs_.size(); i++) {
        outputs_[i].setPosition(2, y);
        window.draw(outputs_[i]);
        y += 15;
        if (i + 1 == MAX_SIZE) {
          break;
        }
      }
    }
  }
}
