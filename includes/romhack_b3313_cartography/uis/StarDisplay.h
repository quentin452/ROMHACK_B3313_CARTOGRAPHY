#pragma once
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
struct StarData {
    std::string courseName;
    int numStars;
    std::string starIcon;
    bool collected = false;

    StarData(const std::string &courseName, int numStars, const std::string &starIcon,bool collected)
        : courseName(courseName), numStars(numStars), starIcon(starIcon),collected(collected) {}
};
class StarDisplay {
  public:
    void afficherEtoilesGroupe(const std::string &groupName, const std::vector<StarData> &stars, sf::RenderWindow &window, const sf::Font &font, int &yOffset);
};