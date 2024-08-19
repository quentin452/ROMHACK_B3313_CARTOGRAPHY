#include <romhack_b3313_cartography/uis/Textures.h>

#include <romhack_b3313_cartography/uis/StarDisplay.h>
#include <romhack_b3313_cartography/utils/rom_utils.h>

void StarDisplay::afficherEtoilesGroupeFusionne(const std::string &groupName, const std::map<std::string, std::vector<StarData>> &courseStarsMap, sf::RenderWindow &window, const sf::Font &font, int &yOffset, float reservedHeight) {
    sf::Text groupText;
    groupText.setFont(font);
    groupText.setString(groupName);
    groupText.setCharacterSize(24);
    groupText.setFillColor(sf::Color::Black);
    groupText.setPosition(100, 100 + yOffset + reservedHeight); // Ajuster pour la hauteur réservée
    window.draw(groupText);

    yOffset += 30;

    std::vector<std::string> courseNames;
    for (const auto &entry : courseStarsMap) {
        courseNames.push_back(entry.first);
    }

    std::sort(courseNames.begin(), courseNames.end());

    for (const auto &courseName : courseNames) {
        const auto &stars = courseStarsMap.at(courseName);

        sf::Text courseText;
        courseText.setFont(font);
        courseText.setString(courseName);
        courseText.setCharacterSize(18);
        courseText.setFillColor(sf::Color::Black);
        sf::FloatRect textBounds = courseText.getLocalBounds();
        courseText.setPosition(100, 130 + yOffset + reservedHeight);
        window.draw(courseText);

        float startX = 100 + textBounds.width + 10;
        float starSpacing = 20.0f;
        float starTextureHeight = std::max(textures.starCollectedTexture.getSize().y, textures.starMissingTexture.getSize().y);

        for (const auto &star : stars) {
            for (int i = 0; i < star.numStars; ++i) {
                sf::Sprite starSprite;
                starSprite.setTexture(star.collected ? textures.starCollectedTexture : textures.starMissingTexture);
                starSprite.setPosition(startX + i * starSpacing, 130 + yOffset + (textBounds.height - starTextureHeight) / 2 + reservedHeight);
                window.draw(starSprite);
            }
            startX += starSpacing;
        }

        yOffset += std::max(30, static_cast<int>(starTextureHeight));
    }

    yOffset += 30;
}