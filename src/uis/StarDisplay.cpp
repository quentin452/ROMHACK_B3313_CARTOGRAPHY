#include <romhack_b3313_cartography/uis/StarDisplay.h>
#include <romhack_b3313_cartography/uis/Textures.h>
#include <romhack_b3313_cartography/utils/rom_utils.h>

void StarDisplay::afficherEtoilesGroupeFusionne(const std::string &groupName, const std::map<std::string, std::vector<StarData>> &courseStarsMap, sf::RenderWindow &window, const sf::Font &font, int &yOffset, float reservedHeight) {
    // Sauvegarder la vue actuelle
    sf::View originalView = window.getView();

    // Définir les dimensions et la position du rectangle
    const float rectWidth = 600;
    const float rectHeight = 400;
    const float rectLeft = 50; // Position fixe
    const float rectTop = 50;  // Position fixe

    // Dessiner le rectangle à une position fixe sur la fenêtre
    sf::RectangleShape rectangle(sf::Vector2f(rectWidth, rectHeight));
    rectangle.setPosition(rectLeft, rectTop);
    rectangle.setFillColor(sf::Color::Transparent);
    rectangle.setOutlineColor(sf::Color::Black);
    rectangle.setOutlineThickness(1);

    // Créer une vue pour dessiner le rectangle en utilisant les coordonnées de la fenêtre
    sf::View fixedView = window.getDefaultView();
    window.setView(fixedView);
    window.draw(rectangle);

    // Restaurer la vue originale
    window.setView(originalView);

    // Dessiner le texte du groupe
    sf::Text groupText;
    groupText.setFont(font);
    groupText.setString(groupName);
    groupText.setCharacterSize(24);
    groupText.setFillColor(sf::Color::Black);
    groupText.setPosition(rectLeft + 100, rectTop + 100 + yOffset + reservedHeight);

    // Vérifiez si le texte du groupe est entièrement dans les limites visibles de la vue
    if (groupText.getGlobalBounds().intersects(sf::FloatRect(originalView.getCenter().x - originalView.getSize().x / 2,
                                                             originalView.getCenter().y - originalView.getSize().y / 2,
                                                             originalView.getSize().x,
                                                             originalView.getSize().y))) {
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
        sf::FloatRect textBounds = courseText.getLocalBounds();
        courseText.setPosition(rectLeft + 100, rectTop + 130 + yOffset + reservedHeight);

        // Vérifiez si le texte du cours est entièrement dans les limites visibles de la vue
        if (courseText.getGlobalBounds().intersects(sf::FloatRect(originalView.getCenter().x - originalView.getSize().x / 2,
                                                                  originalView.getCenter().y - originalView.getSize().y / 2,
                                                                  originalView.getSize().x,
                                                                  originalView.getSize().y))) {
            window.draw(courseText);

            float startX = rectLeft + 100 + textBounds.width + 10;
            float starSpacing = 20.0f;

            for (const auto &star : stars) {
                for (int i = 0; i < star.numStars; ++i) {
                    sf::Sprite starSprite;
                    starSprite.setTexture(star.collected ? textures.starCollectedTexture : textures.starMissingTexture);
                    starSprite.setPosition(startX + i * starSpacing, rectTop + 130 + yOffset + (textBounds.height - starTextureHeight) / 2 + reservedHeight);
                    sf::FloatRect spriteBounds = starSprite.getGlobalBounds();

                    // Vérifiez si le sprite est entièrement dans les limites visibles de la vue
                    if (spriteBounds.intersects(sf::FloatRect(originalView.getCenter().x - originalView.getSize().x / 2,
                                                              originalView.getCenter().y - originalView.getSize().y / 2,
                                                              originalView.getSize().x,
                                                              originalView.getSize().y))) {
                        window.draw(starSprite);
                    }
                }
                startX += starSpacing;
            }
        }

        yOffset += std::max(30, static_cast<int>(starTextureHeight));
    }

    yOffset += 30;
}