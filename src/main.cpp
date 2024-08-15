#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
class Node {
  public:
    Node(float x, float y, const std::string &text, sf::Font &font) : font(font) {
        shape.setRadius(30);
        shape.setFillColor(sf::Color::Cyan);
        shape.setPosition(x - shape.getRadius(), y - shape.getRadius());

        label.setFont(font);
        label.setString(text);
        label.setCharacterSize(12);
        label.setFillColor(sf::Color::Black);
        label.setPosition(x - shape.getRadius() / 2, y - shape.getRadius() / 2);
    }

    void draw(sf::RenderWindow &window) {
        window.draw(shape);
        window.draw(label);
    }

    sf::CircleShape shape;
    sf::Text label;
    sf::Font &font; // Référence à la police
};

#ifdef DEBUG
void addNode(std::vector<Node> &nodes, float x, float y, const std::string &text, sf::Font &font) {
    nodes.emplace_back(x, y, text, font);
}
#endif

int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Mind Map Example");
    sf::Font font;
    if (!font.loadFromFile("Arial.ttf"))
        return 1;

    // Créer des nœuds
    std::vector<Node> nodes;
    nodes.emplace_back(200, 200, "Node 1", font);
    nodes.emplace_back(400, 200, "Node 2", font);
    nodes.emplace_back(300, 400, "Node 3", font);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

#ifdef DEBUG
            if (event.mouseButton.button == sf::Mouse::Right) {
                // Ajouter un nœud au clic droit
                addNode(nodes, event.mouseButton.x, event.mouseButton.y, "New Node", font);
            }
#endif
        }

        window.clear(sf::Color::White);

        // Dessiner les lignes entre les nœuds
        sf::Vertex line1[] = {
            sf::Vertex(sf::Vector2f(200, 200), sf::Color::Black),
            sf::Vertex(sf::Vector2f(400, 200), sf::Color::Black)};

        sf::Vertex line2[] = {
            sf::Vertex(sf::Vector2f(200, 200), sf::Color::Black),
            sf::Vertex(sf::Vector2f(300, 400), sf::Color::Black)};

        sf::Vertex line3[] = {
            sf::Vertex(sf::Vector2f(400, 200), sf::Color::Black),
            sf::Vertex(sf::Vector2f(300, 400), sf::Color::Black)};

        window.draw(line1, 2, sf::Lines);
        window.draw(line2, 2, sf::Lines);
        window.draw(line3, 2, sf::Lines);

        // Dessiner les nœuds
        for (auto &node : nodes) {
            node.draw(window);
        }

        window.display();
    }

    return 0;
}