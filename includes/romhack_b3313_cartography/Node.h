#pragma once

#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

class Node {
  public:
    Node(float x, float y, const std::string &text, sf::Font &font);

    void draw(sf::RenderWindow &window) const;

    void setPosition(float x, float y);

    void setModified(bool status);

    bool isModified() const;

    json toJson() const;

    static Node fromJson(const json &j, sf::Font &font);

    sf::CircleShape star;
    sf::CircleShape shape;
    std::vector<int> connections; // Indices of connected nodes

  private:
    sf::Text label;
    sf::Font &font;
    bool modified;
};