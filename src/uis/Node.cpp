#include <romhack_b3313_cartography/uis/Node.h>

Node::Node(float x, float y, const QString &text, const QFont &font)
    : QGraphicsEllipseItem(x - 30, y - 30, 60, 60),
      label(text), font(font), color(Qt::cyan), modified(false) {
    setRect(x, y, 50, 50);
    setBrush(color);
    setPen(QPen(Qt::black)); // Create a QPen with the desired color
    setFlag(ItemIsMovable);
}

void Node::setPosition(float x, float y) {
    setPos(x - 30, y - 30);
}
void Node::setColor(const QColor &color) {
    this->color = color;
    setBrush(color); // Met à jour la brosse avec la nouvelle couleur
    update();        // Repeint l'élément graphique
}
QJsonObject Node::toJson() const {
    QJsonObject json;
    json["x"] = pos().x() + 30; // Center of ellipse
    json["y"] = pos().y() + 30;
    json["text"] = label;
    return json;
}

Node Node::fromJson(const QJsonObject &json, const QFont &font) {
    float x = json["x"].toDouble();
    float y = json["y"].toDouble();
    QString text = json["text"].toString();
    return Node(x, y, text, font);
}