#include <romhack_b3313_cartography/uis/Node.h>

Node::Node(float x, float y, const QString &text, const QFont &font)
    : QGraphicsEllipseItem(-30, -30, 60, 60),
      label(text), font(font), color(Qt::cyan), modified(false), labelItem(nullptr) {
    setBrush(color);
    setPen(QPen(Qt::cyan));
    setFlag(ItemIsMovable);

    // Positionne le nœud au centre de la position souhaitée
    setPos(x, y);

    // Initialisation du texte
    labelItem = new QGraphicsTextItem(label, this);
    labelItem->setFont(font);
    // Positionne le texte pour qu'il soit centré par rapport à l'ellipse
    labelItem->setPos(-labelItem->boundingRect().width() / 2, -labelItem->boundingRect().height() / 2);
}

void Node::setPosition(float x, float y) {
    modified = true;
    setPos(x, y);
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
    return QGraphicsEllipseItem::boundingRect(); // Utilise le rectangle de l'ellipse
}

void Node::updateStar() {
    if (starItem) {
        delete starItem;
        starItem = nullptr;
    }
    if (modified) {
        QPixmap starPixmap("resources/textures/star-collected.png");
        if (starPixmap.isNull()) {
            qDebug() << "Failed to load star image.";
            return;
        }
        starItem = new QGraphicsPixmapItem(starPixmap, this);
        if (starItem) {
            QPointF starPos = boundingRect().topRight() - QPointF(starPixmap.width(), 0);
            starItem->setPos(starPos);
            qDebug() << "Star position:" << starPos;
        } else {
            qDebug() << "Failed to create QGraphicsPixmapItem.";
        }
    }
}
