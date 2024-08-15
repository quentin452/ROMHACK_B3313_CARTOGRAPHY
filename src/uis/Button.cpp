#include <romhack_b3313_cartography/Button.h>

Button::Button(float x, float y, const sf::Texture &texture) {
    sprite.setTexture(texture);
    sprite.setPosition(x, y);
    boundingBox = sprite.getGlobalBounds();
}

void Button::draw(sf::RenderWindow &window) {
    window.draw(sprite);
}

bool Button::isClicked(sf::Vector2i mousePos) {
    return boundingBox.contains(static_cast<sf::Vector2f>(mousePos));
}
