#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>


class Tab {
public:
    Tab(const std::string &name, float x, float y)
        : tabName(name), posX(x), posY(y), width(100), height(30) {}

    void draw(sf::RenderWindow &window, const sf::Font &font, bool isSelected) const {
        sf::RectangleShape tabRect(sf::Vector2f(width, height)); // Taille des onglets
        tabRect.setPosition(posX, posY);
        if (isSelected) {
            tabRect.setFillColor(sf::Color::Yellow); // Couleur de surbrillance
        } else {
            tabRect.setFillColor(sf::Color::Transparent); // Couleur normale
        }
        tabRect.setOutlineColor(sf::Color::Red);
        tabRect.setOutlineThickness(1);
        window.draw(tabRect);

        sf::Text tabText;
        tabText.setFont(font);
        tabText.setString(tabName);
        tabText.setCharacterSize(24);
        tabText.setFillColor(sf::Color::Black);
        tabText.setPosition(posX + 10, posY); // Ajustez pour centrer le texte
        window.draw(tabText);
    }

    bool contains(const sf::Vector2i &mousePosition) const {
        sf::FloatRect bounds(posX, posY, width, height); // Taille des onglets
        sf::Vector2f mousePosF(static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y));
        return bounds.contains(mousePosF);
    }

    std::string getName() const { return tabName; }

private:
    std::string tabName;
    float posX, posY;
    float width, height; // Dimensions de l'onglet
};
