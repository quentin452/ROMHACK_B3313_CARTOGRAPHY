#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class StarDisplay {
  public:
    StarDisplay();

    void updateStars(const std::vector<bool> &stars);

    void draw(sf::RenderWindow &window);

  private:
    sf::Texture starTexture;
    sf::Sprite starSprite;
    std::vector<sf::Sprite> starSprites;
};