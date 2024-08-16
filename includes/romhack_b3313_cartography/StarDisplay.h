#pragma once
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
struct StarData {
    std::string name;
    int numStars;
    std::string icon;

    StarData(const std::string &name, int numStars, const std::string &icon)
        : name(name), numStars(numStars), icon(icon) {}
};
class StarDisplay {
  public:
    void afficherEtoilesGroupe(const std::string &groupName, const std::vector<StarData> &stars,sf::RenderWindow &window,sf::Font &font);
};