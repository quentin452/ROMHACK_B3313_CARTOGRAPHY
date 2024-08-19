#include <romhack_b3313_cartography/uis/Textures.h>

#include <romhack_b3313_cartography/uis/StarDisplay.h>
#include <romhack_b3313_cartography/utils/rom_utils.h>

void StarDisplay::afficherEtoilesGroupe(const std::string &groupName, const std::vector<StarData> &stars, sf::RenderWindow &window, const sf::Font &font, int &yOffset) {
    sf::Text groupText;
    groupText.setFont(font);
    groupText.setString(groupName);
    groupText.setCharacterSize(24);
    groupText.setFillColor(sf::Color::Black);
    groupText.setPosition(100, 100 + yOffset); // Position du texte du groupe
    window.draw(groupText);

    yOffset += 30; // Espacement après le nom du groupe

    // Espacement pour les étoiles
    float starSpacing = 20.0f; // Espacement entre les étoiles
    float maxTextWidth = 0;    // Largeur maximale pour les noms de cours
    float maxTextHeight = 0;   // Hauteur maximale pour les étoiles

    // Calculer la largeur et la hauteur maximales du texte
    for (const auto &star : stars) {
        sf::Text tempText;
        tempText.setFont(font);
        tempText.setString(star.courseName);
        tempText.setCharacterSize(18);
        sf::FloatRect textBounds = tempText.getLocalBounds();
        if (textBounds.width > maxTextWidth) {
            maxTextWidth = textBounds.width;
        }
        if (textBounds.height > maxTextHeight) {
            maxTextHeight = textBounds.height;
        }
    }

    // Afficher les étoiles pour chaque cours
    for (const auto &star : stars) {
        // Afficher le texte du cours
        sf::Text starText;
        starText.setFont(font);
        starText.setString(star.courseName);
        starText.setCharacterSize(18);
        starText.setFillColor(sf::Color::Black);
        starText.setPosition(100, 130 + yOffset); // Position du texte du cours
        window.draw(starText);

        // Afficher les étoiles en fonction du masque
        float startX = 100 + maxTextWidth + 10; // Position de départ pour les étoiles
        float starTextureHeight = std::max(textures.starCollectedTexture.getSize().y, textures.starMissingTexture.getSize().y);

        for (int i = 0; i < star.numStars; ++i) {
            sf::Sprite starSprite;
            bool isCollected = (star.mask & (1 << i)) != 0; // Vérifie si l'étoile i est collectée
            starSprite.setTexture(isCollected ? textures.starCollectedTexture : textures.starMissingTexture);
            starSprite.setPosition(startX + i * starSpacing, 130 + yOffset + (maxTextHeight - starTextureHeight) / 2);
            window.draw(starSprite);
        }

        yOffset += std::max(30, static_cast<int>(starTextureHeight)); // Espacement entre les étoiles, ajusté pour la hauteur de la texture
    }

    yOffset += 30; // Espacement après le groupe
}