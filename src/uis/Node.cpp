#include <romhack_b3313_cartography/uis/Node.h>

Node::Node(float x, float y, const QString &text, const QFont &font)
    : QGraphicsEllipseItem(x - 25, y - 25, 50, 50), // Centrer le nœud
      label(text), font(font), color(Qt::cyan), modified(false) {
    setBrush(color);
    setPen(QPen(Qt::black)); // Crée un QPen avec la couleur souhaitée
    setFlag(ItemIsMovable);
}

void Node::setMovable(bool movable) {
    setFlag(ItemIsMovable, movable); // Définit le drapeau ItemIsMovable en fonction du paramètre
}
void Node::setColor(const QColor &color) {
    this->color = color;
    setBrush(color); // Met à jour la brosse avec la nouvelle couleur
    update();        // Repeint l'élément graphique
}
QJsonObject Node::toJson() const {
    QJsonObject json;
    json["x"] = pos().x();
    json["y"] = pos().y();
    json["text"] = label;
    return json;
}

Node Node::fromJson(const QJsonObject &json, const QFont &font) {
    float x = json["x"].toDouble();
    float y = json["y"].toDouble();
    QString text = json["text"].toString();
    return Node(x, y, text, font);
}
void Node::addConnection(int nodeIndex) {
    if (!connections.contains(nodeIndex)) {
        connections.append(nodeIndex);
    }
}

void Node::removeConnection(int nodeIndex) {
    connections.removeAll(nodeIndex);
}
QRectF Node::boundingRect() const {
    return labelItem->boundingRect(); // Ajuster en fonction des besoins
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    // Dessine le nœud
    painter->drawRect(boundingRect());
    // Dessine l'étiquette
    labelItem->paint(painter, option, widget);
}

void Node::updateStar() {
    if (starItem) {
        delete starItem;
        starItem = nullptr;
    }
    if (modified) {
        QPixmap starPixmap("resources/textures/star-missing.png"); // Remplace par le chemin de ton image d'étoile
        starItem = new QGraphicsPixmapItem(starPixmap, this);
        starItem->setPos(boundingRect().topRight() - QPointF(starPixmap.width(), 0)); // Positionne l'étoile en haut à droite
    }
}