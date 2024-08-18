#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class DropdownMenu {
  public:
    DropdownMenu(float x, float y, const sf::Texture &texture, const std::vector<std::string> &items, sf::Font &font);
    void draw(sf::RenderWindow &window);

    bool isClicked(sf::Vector2i mousePos);
    std::string getSelectedItem(sf::Vector2i mousePos);

  private:
    sf::Sprite background;
    sf::Font font;
    std::vector<sf::Text> itemTexts;
    std::vector<std::string> items;
    sf::Vector2f position;
};
