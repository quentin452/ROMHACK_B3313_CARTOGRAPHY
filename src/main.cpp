#include <SFML/Graphics.hpp>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include <romhack_b3313_cartography/Button.h>
#include <romhack_b3313_cartography/DropdownMenu.h>
#include <romhack_b3313_cartography/Node.h>

using json = nlohmann::json;

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

    // Charger textures
    sf::Texture saveTexture;
    if (!saveTexture.loadFromFile("resources/textures/save_icon.png"))
        return 1;

    sf::Texture dropdownTexture;
    if (!dropdownTexture.loadFromFile("resources/textures/dropdown_background.png"))
        return 1;

    // Créer les objets
    Button saveButton(1150, 10, saveTexture); // Positionnez le bouton comme vous le souhaitez

    std::vector<std::string> menuItems = {"b3313-v1.0.2.json", "another_file.json"};

    sf::Font font;
    if (!font.loadFromFile("resources/assets/Arial.ttf"))
        return 1;

    DropdownMenu dropdownMenu(1150, 50, dropdownTexture, menuItems, font); // Positionnez le menu comme vous le souhaitez

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
                    if (saveButton.isClicked(sf::Vector2i(event.mouseButton.x, event.mouseButton.y))) {
                        saveNodes(nodes, "b3313-v1.0.2.json");
                    } else if (dropdownMenu.isClicked(sf::Vector2i(event.mouseButton.x, event.mouseButton.y))) {
                        std::string selectedFile = dropdownMenu.getSelectedItem(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                        if (!selectedFile.empty()) {
                            nodes = loadNodes(selectedFile, font);
                        }
                    } else if (isMouseOverNode(nodes, sf::Vector2i(event.mouseButton.x, event.mouseButton.y), startNodeIndex)) {
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

        // Draw button and menu
        saveButton.draw(window);
        dropdownMenu.draw(window);

        window.display();
    }

    return 0;
}