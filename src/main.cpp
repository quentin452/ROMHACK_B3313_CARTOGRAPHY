#include <SFML/Graphics.hpp>
#include <vector>

class Node {
public:
    Node(float x, float y, const std::string& text) {
        shape.setRadius(30);
        shape.setFillColor(sf::Color::Cyan);
        shape.setPosition(x - shape.getRadius(), y - shape.getRadius());

        if (!font.loadFromFile("Arial.ttf")) return;

        label.setFont(font);
        label.setString(text);
        label.setCharacterSize(12);
        label.setFillColor(sf::Color::Black);
        label.setPosition(x - shape.getRadius() / 2, y - shape.getRadius() / 2);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
        window.draw(label);
    }

    sf::CircleShape shape;
    sf::Text label;
    sf::Font font;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Mind Map Example");

    // Créer des nœuds
    std::vector<Node> nodes;
    nodes.emplace_back(200, 200, "Node 1");
    nodes.emplace_back(400, 200, "Node 2");
    nodes.emplace_back(300, 400, "Node 3");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        window.clear(sf::Color::White);

        // Dessiner les lignes entre les nœuds
        sf::Vertex line1[] = {
            sf::Vertex(sf::Vector2f(200, 200), sf::Color::Black),
            sf::Vertex(sf::Vector2f(400, 200), sf::Color::Black)
        };

        sf::Vertex line2[] = {
            sf::Vertex(sf::Vector2f(200, 200), sf::Color::Black),
            sf::Vertex(sf::Vector2f(300, 400), sf::Color::Black)
        };

        sf::Vertex line3[] = {
            sf::Vertex(sf::Vector2f(400, 200), sf::Color::Black),
            sf::Vertex(sf::Vector2f(300, 400), sf::Color::Black)
        };

        window.draw(line1, 2, sf::Lines);
        window.draw(line2, 2, sf::Lines);
        window.draw(line3, 2, sf::Lines);

        // Dessiner les nœuds
        for (auto& node : nodes) {
            node.draw(window);
        }

        window.display();
    }

    return 0;
}
