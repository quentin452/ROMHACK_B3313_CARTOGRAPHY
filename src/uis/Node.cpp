#include <romhack_b3313_cartography/uis/Node.h>

Node::Node(float x, float y, const QString &text, const QFont &font, NodeShapes shape)
    : QGraphicsEllipseItem(-30, -30, 60, 60),
      label(text), font(font), color(Qt::cyan), shape(shape), modified(false), labelItem(nullptr) {
    setBrush(color);
    setPen(QPen(Qt::cyan));
    setFlag(ItemIsMovable);
    setPos(x, y);
    labelItem = new QGraphicsTextItem(label, this);
    labelItem->setFont(font);
    labelItem->setPos(-labelItem->boundingRect().width() / 2, -labelItem->boundingRect().height() / 2);
    m_name = labelItem->toPlainText();
}

void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsEllipseItem::mouseReleaseEvent(event);
    if (flags() & ItemIsMovable)
        setModified(true);
}

void Node::setMovable(bool movable) {
    setFlag(ItemIsMovable, movable);
}

void Node::setName(const QString &name) {
    m_name = name;
    if (labelItem) {
        labelItem->setPlainText(m_name);
        labelItem->setPos(-labelItem->boundingRect().width() / 2, -labelItem->boundingRect().height() / 2);
    }
}

void Node::setColor(const QColor &color) {
    this->color = color;
    setBrush(color);
    update();
}

void Node::setShape(NodeShapes newShape) {
    shape = newShape;
    update(); // Schedule a redraw of the item
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);

    QRectF rect = boundingRect().adjusted(1, 1, -1, -1); 

    switch (shape) {
    case Circle:
        painter->drawEllipse(rect);
        break;
    case Square:
        painter->drawRect(rect);
        break;
    case Triangle: {
        QPolygonF triangle;
        triangle << QPointF(rect.center().x(), rect.top())
                 << QPointF(rect.left(), rect.bottom())
                 << QPointF(rect.right(), rect.bottom());
        painter->drawPolygon(triangle);
        break;
    }
    }
    if (labelItem) 
        labelItem->setPos(-labelItem->boundingRect().width() / 2, -labelItem->boundingRect().height() / 2);
}

QString Node::shapeToString(NodeShapes shape) const {
    switch (shape) {
    case Circle:
        return "Circle";
    case Square:
        return "Square";
    case Triangle:
        return "Triangle";
    default:
        return "Unknown";
    }
}

QJsonObject Node::toJson() const {
    QJsonObject json;
    json["x"] = pos().x();
    json["y"] = pos().y();
    json["text"] = m_name;
    json["font_size"] = font.pointSize();

    json["shape"] = shapeToString(shape);

    QJsonArray connectionsArray;
    for (int conn : connections) {
        connectionsArray.append(conn);
    }
    json["connections"] = connectionsArray;
    return json;
}

Node *Node::fromJson(const QJsonObject &json, const QFont &defaultFont) {
    float x = json["x"].toDouble();
    float y = json["y"].toDouble();
    QString text = json["text"].toString();
    QFont font = defaultFont;
    int fontSize = json["font_size"].toInt(font.pointSize());
    font.setPointSize(fontSize);

    NodeShapes shape = Circle; // Default value

    if (json.contains("shape")) {
        QString shapeString = json["shape"].toString();
        shape = stringToShape(shapeString);
    }

    Node *node = new Node(x, y, text, font, shape);

    if (json.contains("connections") && json["connections"].isArray()) {
        QJsonArray connectionsArray = json["connections"].toArray();
        for (const QJsonValue &value : connectionsArray) {
            if (value.isDouble()) {
                node->addConnection(value.toInt());
            }
        }
    }
    return node;
}

void Node::addConnection(int nodeIndex) {
    if (!connections.contains(nodeIndex))
        connections.append(nodeIndex);
}

void Node::removeConnection(int nodeIndex) {
    connections.removeAll(nodeIndex);
}

void Node::setModified(bool modified) {
    this->modified = modified;
    updateIsModified();
}

void Node::updateIsModified() {
    for (QGraphicsPixmapItem *starItem : starItems) {
        if (scene() && scene()->items().contains(starItem)) {
            scene()->removeItem(starItem);
            delete starItem;
        }
    }
    starItems.clear();
    if (modified) {
        QPixmap starPixmap("resources/textures/star-collected.png");
        if (starPixmap.isNull()) {
            qDebug() << "Failed to load star image.";
            return;
        }
        QGraphicsPixmapItem *starItem = new QGraphicsPixmapItem(starPixmap, this);
        if (starItem) {
            starItem->setScale(1.0 / 2.0);
            QPointF starPos = boundingRect().topRight() - QPointF(starPixmap.width() / 3.0, 0);
            starItem->setPos(starPos);
            starItems.append(starItem);
        } else {
            qDebug() << "Failed to create QGraphicsPixmapItem.";
        }
    }
}