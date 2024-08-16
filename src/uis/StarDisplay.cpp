#include <romhack_b3313_cartography/StarDisplay.h>
StarDisplay::StarDisplay() {
    // Initialisation de la texture des étoiles si nécessaire
    if (!starTexture.loadFromFile("resources/textures/star.png")) {
        // Gérer l'erreur de chargement
    }
    starSprite.setTexture(starTexture);
}

void StarDisplay::updateStars(const std::vector<bool> &stars) {
    starSprites.clear();
    for (size_t i = 0; i < stars.size(); ++i) {
        if (stars[i]) {
            sf::Sprite sprite(starSprite);
            // Positionner les étoiles ici (à adapter selon vos besoins)
            // Exemple :
            sprite.setPosition((i % 20) * 32.f, (i / 20) * 32.f); // Ajustez la grille selon votre besoin
            starSprites.push_back(sprite);
        }
    }
}

void StarDisplay::draw(sf::RenderWindow &window) {
    for (const auto &sprite : starSprites) {
        window.draw(sprite);
    }
}