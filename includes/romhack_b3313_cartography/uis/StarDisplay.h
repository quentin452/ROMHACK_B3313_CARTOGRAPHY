#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

struct StarData {
    std::string courseName;
    int numStars;
    bool collected = false;
    int offset; // Ajouté pour stocker l'offset
    int mask;   // Ajouté pour stocker le mask
    StarData(const std::string &courseName, int numStars, bool collected, int offset, int mask)
        : courseName(courseName), numStars(numStars), collected(collected), offset(offset), mask(mask) {}
};

class StarDisplay {
  public:
    void afficherEtoilesGroupe(const std::string &groupName, const std::vector<StarData> &stars, sf::RenderWindow &window, const sf::Font &font, int &yOffset);
};
