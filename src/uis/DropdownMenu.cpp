#include <romhack_b3313_cartography/uis/DropdownMenu.h>

DropdownMenu::DropdownMenu(float x, float y, const sf::Texture &texture, const std::vector<std::string> &items, sf::Font &font) : font(font), position(x, y), items(items) {
    background.setTexture(texture);
    background.setPosition(x, y);

    for (size_t i = 0; i < items.size(); ++i) {
        sf::Text text;
        text.setFont(font);
        text.setString(items[i]);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::Black);
        text.setPosition(x, y + i * 30); // Adjust position as needed
        itemTexts.push_back(text);
    }
}

void DropdownMenu::draw(sf::RenderWindow &window) {
    window.draw(background);
    for (const auto &text : itemTexts) {
        window.draw(text);
    }
}

bool DropdownMenu::isClicked(sf::Vector2i mousePos) {
    // Check if the mouse is over the dropdown menu
    return background.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}

std::string DropdownMenu::getSelectedItem(sf::Vector2i mousePos) {
    for (size_t i = 0; i < itemTexts.size(); ++i) {
        if (itemTexts[i].getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos))) {
            return items[i];
        }
    }
    return "";
}