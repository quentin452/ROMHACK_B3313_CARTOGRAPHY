#include <romhack_b3313_cartography/Textures.h>

#include <romhack_b3313_cartography/StarDisplay.h>
void StarDisplay::afficherEtoilesGroupe(const std::string &groupName, const std::vector<StarData> &stars, sf::RenderWindow &window, const sf::Font &font, int &yOffset) {
    sf::Text groupText;
    groupText.setFont(font);
    groupText.setString(groupName);
    groupText.setCharacterSize(24);
    groupText.setFillColor(sf::Color::Black);
    groupText.setPosition(100, 100 + yOffset); // Position du texte du groupe
    window.draw(groupText);

    yOffset += 30; // Espacement après le nom du groupe

    // Calculer la largeur et la hauteur maximales du texte
    float maxTextWidth = 0;
    float maxTextHeight = 0;
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

    // Calculer la hauteur de la texture des étoiles
    float starTextureHeight = 0;
    if (textures.starCollectedTexture.getSize().y > textures.starMissingTexture.getSize().y) {
        starTextureHeight = textures.starCollectedTexture.getSize().y;
    } else {
        starTextureHeight = textures.starMissingTexture.getSize().y;
    }

    for (const auto &star : stars) {
        // Afficher le texte du cours
        sf::Text starText;
        starText.setFont(font);
        starText.setString(star.courseName);
        starText.setCharacterSize(18);
        starText.setFillColor(sf::Color::Black);
        starText.setPosition(100, 130 + yOffset); // Position du texte du cours
        window.draw(starText);

        // Afficher les étoiles à droite du texte avec espacement
        sf::Sprite starSprite;
        if (star.numStars > 0) {
            starSprite.setTexture(textures.starCollectedTexture);
        } else {
            starSprite.setTexture(textures.starMissingTexture);
        }
        float starSpacing = 20.0f; // Espacement entre les étoiles
        starSprite.setPosition(100 + maxTextWidth + 10 + starSpacing, 130 + yOffset + (maxTextHeight - starTextureHeight) / 2); // Centrer verticalement les étoiles par rapport au texte
        window.draw(starSprite);

        yOffset += std::max(30, static_cast<int>(starTextureHeight)); // Espacement entre les étoiles, ajusté pour la hauteur de la texture
    }

    yOffset += 30; // Espacement après le groupe
}