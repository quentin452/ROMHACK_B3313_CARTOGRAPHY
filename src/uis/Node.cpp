#include <romhack_b3313_cartography/Node.h>

Node::Node(float x, float y, const std::string &text, sf::Font &font) : font(font), modified(true) {
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

void Node::draw(sf::RenderWindow &window) const {
    window.draw(shape);
    window.draw(label);
    if (modified)
        window.draw(star);
}

void Node::setPosition(float x, float y) {
    shape.setPosition(x - shape.getRadius(), y - shape.getRadius());
    label.setPosition(x - shape.getRadius() / 2, y - shape.getRadius() / 2);
    star.setPosition(x + shape.getRadius() - 10, y - shape.getRadius() - 10);
    modified = true;
}

void Node::setModified(bool status) {
    modified = status;
}

bool Node::isModified() const {
    return modified;
}

json Node::toJson() const {
    return json{{"x", shape.getPosition().x + shape.getRadius()},
                {"y", shape.getPosition().y + shape.getRadius()},
                {"text", label.getString().toAnsiString()}};
}

Node Node::fromJson(const json &j, sf::Font &font) {
    Node node(j["x"], j["y"], j["text"], font);
    node.setModified(false); // Mark as not modified after loading
    return node;
}
