#include <SFML/Graphics.hpp>
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

    void draw(sf::RenderWindow &window) const {
        window.draw(shape);
        window.draw(label);
    }

  private:
    sf::CircleShape shape;
    sf::Text label;
    sf::Font &font;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Mind Map Example");

    sf::Font font;
    if (!font.loadFromFile("Arial.ttf"))
        return 1;

    std::vector<Node> nodes = {
        Node(200, 200, "Node 1", font),
        Node(400, 200, "Node 2", font),
        Node(300, 400, "Node 3", font)};

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
#ifdef DEBUG
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
                nodes.emplace_back(event.mouseButton.x, event.mouseButton.y, "New Node", font);
            }
#endif
        }

        window.clear(sf::Color::White);

        // Dessiner les lignes entre les nœuds
        sf::Vertex lines[] = {
            sf::Vertex(sf::Vector2f(200, 200), sf::Color::Black),
            sf::Vertex(sf::Vector2f(400, 200), sf::Color::Black),
            sf::Vertex(sf::Vector2f(200, 200), sf::Color::Black),
            sf::Vertex(sf::Vector2f(300, 400), sf::Color::Black),
            sf::Vertex(sf::Vector2f(400, 200), sf::Color::Black),
            sf::Vertex(sf::Vector2f(300, 400), sf::Color::Black)};
        window.draw(lines, 6, sf::Lines);

        // Dessiner les nœuds
        for (const auto &node : nodes) {
            node.draw(window);
        }

        window.display();
    }

    return 0;
}