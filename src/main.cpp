#include <SFML/Graphics.hpp>
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <chrono>
#include <ctime>

using json = nlohmann::json;

class Node {
  public:
    Node(float x, float y, const std::string &text, sf::Font &font) : font(font), modified(true) {
        shape.setRadius(30);
        shape.setFillColor(sf::Color::Cyan);
        shape.setPosition(x - shape.getRadius(), y - shape.getRadius());

        label.setFont(font);
        label.setString(text);
        label.setCharacterSize(12);
        label.setFillColor(sf::Color::Black);
        label.setPosition(x - shape.getRadius() / 2, y - shape.getRadius() / 2);

        star.setRadius(5);
        star.setFillColor(sf::Color::Red);
        star.setPosition(x + shape.getRadius() - 10, y - shape.getRadius() - 10);
    }

    void draw(sf::RenderWindow &window) const {
        window.draw(shape);
        window.draw(label);
        if (modified)
            window.draw(star);
    }

    void setPosition(float x, float y) {
        shape.setPosition(x - shape.getRadius(), y - shape.getRadius());
        label.setPosition(x - shape.getRadius() / 2, y - shape.getRadius() / 2);
        star.setPosition(x + shape.getRadius() - 10, y - shape.getRadius() - 10);
        modified = true;
    }

    void setModified(bool status) {
        modified = status;
    }

    bool isModified() const {
        return modified;
    }

    json toJson() const {
        return json{{"x", shape.getPosition().x + shape.getRadius()},
                    {"y", shape.getPosition().y + shape.getRadius()},
                    {"text", label.getString().toAnsiString()}};
    }

    static Node fromJson(const json &j, sf::Font &font) {
        Node node(j["x"], j["y"], j["text"], font);
        node.setModified(false); // Mark as not modified after loading
        return node;
    }

    sf::CircleShape star;
    sf::CircleShape shape;
    std::vector<int> connections; // Indices of connected nodes

  private:
    sf::Text label;
    sf::Font &font;
    bool modified;
};

void saveNodes(const std::vector<Node> &nodes, const std::string &filename) {
    json j;
    for (const auto &node : nodes) {
        j.push_back(node.toJson());
    }
    std::ofstream file(filename);
    file << j.dump(4); // Pretty print JSON with an indent of 4 spaces
}

std::vector<Node> loadNodes(const std::string &filename, sf::Font &font) {
    std::ifstream file(filename);

    // Vérifier si le fichier a été ouvert avec succès
    if (!file.is_open()) {
        return {}; // Retourner un vecteur vide si le fichier n'existe pas
    }

    json j;
    file >> j;

    std::vector<Node> nodes;
    for (const auto &item : j) {
        nodes.push_back(Node::fromJson(item, font));
    }
    return nodes;
}
bool isMouseOverNode(const std::vector<Node> &nodes, sf::Vector2i mousePos, int &nodeIndex) {
    for (int i = 0; i < nodes.size(); ++i) {
        sf::Vector2f nodePos = nodes[i].shape.getPosition();
        sf::FloatRect nodeBounds(nodePos.x, nodePos.y, nodes[i].shape.getRadius() * 2, nodes[i].shape.getRadius() * 2);
        if (nodeBounds.contains(static_cast<sf::Vector2f>(mousePos))) {
            nodeIndex = i;
            return true;
        }
    }
    return false;
}
int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Mind Map Example");
    sf::Font font;
    if (!font.loadFromFile("Arial.ttf"))
        return 1;

    std::vector<Node> nodes = loadNodes("b3313-v1.0.2.json", font);
    sf::Vector2i startPos;
    bool dragging = false;
    int startNodeIndex = -1;
    std::vector<std::pair<int, int>> connections;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    if (isMouseOverNode(nodes, sf::Vector2i(event.mouseButton.x, event.mouseButton.y), startNodeIndex)) {
                        startPos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
                        dragging = true;
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left && dragging) {
                    int endNodeIndex = -1;
                    if (isMouseOverNode(nodes, sf::Vector2i(event.mouseButton.x, event.mouseButton.y), endNodeIndex) && endNodeIndex != startNodeIndex) {
                        connections.push_back({startNodeIndex, endNodeIndex});
                        nodes[startNodeIndex].connections.push_back(endNodeIndex);
                        nodes[endNodeIndex].connections.push_back(startNodeIndex);
                    }

                    dragging = false;
                }
            }
            if (event.type == sf::Event::MouseMoved && dragging) {
                // Handle dragging logic if needed
            }
#ifdef DEBUG
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Right) {
                nodes.emplace_back(event.mouseButton.x, event.mouseButton.y, "New Node", font);
            }
#endif
        }

        window.clear(sf::Color::White);

        // Draw connections
        for (const auto &conn : connections) {
            sf::Vertex line[] = {
                sf::Vertex(nodes[conn.first].shape.getPosition() + sf::Vector2f(30, 30), sf::Color::Black),
                sf::Vertex(nodes[conn.second].shape.getPosition() + sf::Vector2f(30, 30), sf::Color::Black)};
            window.draw(line, 2, sf::Lines);

            // Draw arrow
            sf::Vector2f dir = nodes[conn.second].shape.getPosition() - nodes[conn.first].shape.getPosition();
            float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            dir /= length;

            float arrowSize = 10.f;
            sf::Vector2f arrowPoint1 = nodes[conn.second].shape.getPosition() - dir * (arrowSize + 10.f) + sf::Vector2f(-dir.y * arrowSize, dir.x * arrowSize);
            sf::Vector2f arrowPoint2 = nodes[conn.second].shape.getPosition() - dir * (arrowSize + 10.f) + sf::Vector2f(dir.y * arrowSize, -dir.x * arrowSize);

            sf::Vertex arrow[] = {
                sf::Vertex(nodes[conn.second].shape.getPosition(), sf::Color::Black),
                sf::Vertex(arrowPoint1, sf::Color::Black),
                sf::Vertex(arrowPoint2, sf::Color::Black)};

            window.draw(arrow, 3, sf::Triangles);
        }

        // Draw nodes
        for (const auto &node : nodes) {
            node.draw(window);
        }

        window.display();
    }

    saveNodes(nodes, "b3313-v1.0.2.json");

    return 0;
}