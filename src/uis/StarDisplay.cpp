#include <iostream> // Pour les logs
#include <romhack_b3313_cartography/uis/StarDisplay.h>
#include <romhack_b3313_cartography/uis/Textures.h>
#include <romhack_b3313_cartography/utils/rom_utils.h>

void StarDisplay::afficherEtoilesGroupeFusionne(const std::string &groupName, const std::map<std::string, std::vector<StarData>> &courseStarsMap, sf::RenderWindow &window, const sf::Font &font, int &yOffset, float reservedHeight) {
    // Sauvegarder la vue originale
    sf::View originalView = window.getView();
    sf::Vector2u windowSize = window.getSize();

    // Définir les dimensions et la position du rectangle fixe
    const float rectWidth = 600;
    const float rectLeft = 50;
    const float rectTop = 50;
    const float rectHeight = windowSize.y - rectTop; // Ajuste la hauteur du rectangle pour qu'il s'étende jusqu'en bas de la fenêtre

    // Dessiner le rectangle fixe en utilisant la vue principale
    window.setView(window.getDefaultView());
    sf::RectangleShape rectangle(sf::Vector2f(rectWidth, rectHeight));
    rectangle.setPosition(rectLeft, rectTop);
    rectangle.setFillColor(sf::Color::Transparent);
    rectangle.setOutlineColor(sf::Color::Black);
    rectangle.setOutlineThickness(1);
    window.draw(rectangle);

    // Restaurer la vue originale pour dessiner les éléments
    window.setView(originalView);

    // Définir les limites du rectangle fixe en coordonnées de la vue actuelle
    sf::FloatRect rectBounds(
        rectLeft - originalView.getCenter().x + originalView.getSize().x / 2,
        rectTop - originalView.getCenter().y + originalView.getSize().y / 2,
        rectWidth,
        rectHeight
    );

    // Log des limites du rectangle fixe
    std::cout << "Rectangle Bounds: " << rectBounds.left << ", " << rectBounds.top << ", " << rectBounds.width << ", " << rectBounds.height << std::endl;

    // Dessiner le texte du groupe
    sf::Text groupText;
    groupText.setFont(font);
    groupText.setString(groupName);
    groupText.setCharacterSize(24);
    groupText.setFillColor(sf::Color::Black);
    groupText.setPosition(rectLeft + 100, rectTop + 100 + yOffset + reservedHeight);

    // Vérifiez si le texte du groupe est dans les limites du rectangle
    if (rectBounds.intersects(groupText.getGlobalBounds())) {
        window.draw(groupText);
    }

    yOffset += 30;

    // Préparer les noms des cours
    std::vector<std::string> courseNames;
    for (const auto &entry : courseStarsMap) {
        courseNames.push_back(entry.first);
    }
    std::sort(courseNames.begin(), courseNames.end());

    // Calculer la hauteur de la texture des étoiles
    float starTextureHeight = std::max(textures.starCollectedTexture.getSize().y, textures.starMissingTexture.getSize().y);

    // Dessiner les éléments pour chaque cours
    for (const auto &courseName : courseNames) {
        const auto &stars = courseStarsMap.at(courseName);

        sf::Text courseText;
        courseText.setFont(font);
        courseText.setString(courseName);
        courseText.setCharacterSize(18);
        courseText.setFillColor(sf::Color::Black);
        courseText.setPosition(rectLeft + 100, rectTop + 130 + yOffset + reservedHeight);

        // Vérifiez si le texte du cours est dans les limites du rectangle
        if (rectBounds.intersects(courseText.getGlobalBounds())) {
            window.draw(courseText);

            float startX = rectLeft + 100 + courseText.getLocalBounds().width + 10;
            float starSpacing = 20.0f;

            for (const auto &star : stars) {
                for (int i = 0; i < star.numStars; ++i) {
                    sf::Sprite starSprite;
                    starSprite.setTexture(star.collected ? textures.starCollectedTexture : textures.starMissingTexture);
                    starSprite.setPosition(startX + i * starSpacing, rectTop + 130 + yOffset + (courseText.getLocalBounds().height - starTextureHeight) / 2 + reservedHeight);
                    // Vérifiez si le sprite est dans les limites du rectangle
                    if (rectBounds.intersects(starSprite.getGlobalBounds())) {
                        window.draw(starSprite);
                    }
                }
                startX += starSpacing;
            }
        }

        yOffset += std::max(30, static_cast<int>(starTextureHeight));
    }

    yOffset += 30;

    // Restaurer la vue originale
    window.setView(originalView);
}
