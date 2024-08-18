#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <string>
#include <vector>


class Button {
  public:
    Button(float x, float y, const sf::Texture &texture);

    void draw(sf::RenderWindow &window);

    bool isClicked(sf::Vector2i mousePos);

  private:
    sf::Sprite sprite;
    sf::FloatRect boundingBox;
};