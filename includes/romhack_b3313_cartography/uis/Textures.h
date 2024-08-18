#ifndef TEXTURES_H
#define TEXTURES_H
#include <SFML/Graphics.hpp>

struct Textures {
    sf::Texture saveTexture;
    sf::Texture starCollectedTexture;
    sf::Texture starMissingTexture;
    sf::Texture dropdownTexture;

    bool loadTextures() {
        if (!saveTexture.loadFromFile("resources/textures/save_icon.png"))
            return false;
        if (!starCollectedTexture.loadFromFile("resources/textures/star-collected.png"))
            return false;
        if (!starMissingTexture.loadFromFile("resources/textures/star-missing.png"))
            return false;
        if (!dropdownTexture.loadFromFile("resources/textures/dropdown_background.png"))
            return false;
        return true;
    }
};

extern Textures textures;
#endif // TEXTURES_H