#include <romhack_b3313_cartography/StarDisplay.h>

void StarDisplay::afficherEtoilesGroupe(const std::string &groupName, const std::vector<StarData> &stars,sf::RenderWindow &window,sf::Font &font) {
    // Affichage des étoiles en fonction des données
    for (const auto &star : stars) {
        sf::Text starText;
        starText.setFont(font);
        starText.setString(star.icon);
        starText.setCharacterSize(24);  // Taille des étoiles
        starText.setPosition(100, 100); // Position à adapter selon vos besoins
        window.draw(starText);
    }
}