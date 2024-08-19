#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <algorithm>

struct StarData {
    std::string courseName;
    int numStars;
    bool collected = false;
    int offset; // Ajouté pour stocker l'offset
    int mask;   // Ajouté pour stocker le mask

    // Constructeur par défaut
    StarData() : numStars(0), collected(false), offset(0), mask(0) {}

    // Constructeur avec paramètres
    StarData(const std::string &courseName, int numStars, bool collected, int offset, int mask)
        : courseName(courseName), numStars(numStars), collected(collected), offset(offset), mask(mask) {}
};

class StarDisplay {
  public:
    void afficherEtoilesGroupeFusionne(const std::string &groupName, const std::map<std::string, std::vector<StarData>> &courseStarsMap, sf::RenderWindow &window, const sf::Font &font, int &yOffset, float reservedHeight);
};
