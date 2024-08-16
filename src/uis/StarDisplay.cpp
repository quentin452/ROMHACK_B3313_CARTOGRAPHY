#include <romhack_b3313_cartography/Textures.h>

#include <romhack_b3313_cartography/StarDisplay.h>

void StarDisplay::afficherEtoilesGroupe(const std::string &groupName, const std::vector<StarData> &stars, sf::RenderWindow &window, const sf::Font &font) {
    sf::Text groupText;
    groupText.setFont(font);
    groupText.setString(groupName);
    groupText.setCharacterSize(24);
    groupText.setFillColor(sf::Color::Black);
    groupText.setPosition(100, 100); // Ajustez la position selon vos besoins
    window.draw(groupText);

    int yOffset = 0;
    for (const auto &star : stars) {
        sf::Sprite starSprite;
        if (star.numStars > 0) {
            starSprite.setTexture(textures.starCollectedTexture);
        } else {
            starSprite.setTexture(textures.starMissingTexture);
        }
        starSprite.setPosition(100, 130 + yOffset); // Ajustez la position selon vos besoins
        window.draw(starSprite);

        sf::Text starText;
        starText.setFont(font);
        starText.setString(star.courseName);
        starText.setCharacterSize(18);
        starText.setFillColor(sf::Color::Black);
        starText.setPosition(150, 130 + yOffset); // Ajustez la position selon vos besoins
        window.draw(starText);

        yOffset += 30; // Espacement entre les étoiles
    }
}
