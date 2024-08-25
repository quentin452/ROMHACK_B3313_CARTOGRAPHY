#include <romhack_b3313_cartography/uis/Node.h>

#include "../windows/MainWindow.h"
QList<QGraphicsPixmapItem *> Node::starItems;
Node::Node(float x, float y, const QString &text, const QFont &font, NodeShapes shape)
    : QGraphicsEllipseItem(-30, -30, 60, 60),
      label(text), font(font), color(Qt::cyan), shape(shape), modified(false), labelItem(nullptr), starAssociated(false) {
    setBrush(color);
    setPen(QPen(Qt::cyan));
    setFlag(ItemIsMovable);
    setPos(x, y);
    labelItem = new QGraphicsTextItem(label, this);
    labelItem->setFont(font);
    m_name = labelItem->toPlainText();
    adjustNodeSize();
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
        adjustNodeSize();
    }
}
void Node::adjustNodeSize() {
    if (labelItem) {
        QPointF currentPos = pos();
        QRectF labelTextRect = labelItem->boundingRect();
        QRectF associatedCourseTextRect;

        if (!associatedCourse.isEmpty()) {
            QGraphicsTextItem tempItem;
            tempItem.setPlainText(associatedCourse);
            tempItem.setFont(font);
            associatedCourseTextRect = tempItem.boundingRect();
        }

        float padding = 10.0;
        float width = qMax(labelTextRect.width(), associatedCourseTextRect.width()) + padding;
        float height = labelTextRect.height() + associatedCourseTextRect.height() + padding;

        prepareGeometryChange();

        if (shape == Triangle) {
            float triangleHeight = height * sqrt(3) / 2;
            float triangleWidth = width * sqrt(3);
            setRect(-triangleWidth / 2, -triangleHeight / 2, triangleWidth, triangleHeight);
            labelItem->setPos(-labelTextRect.width() / 2, -labelTextRect.height() / 2 + 5);
        } else {
            setRect(-width / 2, -height / 2, width, height);
            labelItem->setPos(-labelTextRect.width() / 2, -labelTextRect.height() / 2);
        }

        setPos(currentPos);
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
void Node::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (!isShiftPressed() && (QApplication::keyboardModifiers() & Qt::ControlModifier) && event->button() == Qt::LeftButton) {
        if (isStarAssociated()) {
            MainWindow::jump_to_star_display_associated_line = true;
            MainWindow::jump_to_which_line = getAssociatedCourse();
            // MainWindow::graphicsView->centerOn(this->pos());
            MainWindow::force_toggle_star_display = true;
        }
    }
    QGraphicsItem::mousePressEvent(event);
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

    if (!associatedCourse.isEmpty()) {
        painter->setPen(Qt::black);
        painter->drawText(rect, Qt::AlignBottom | Qt::AlignHCenter, associatedCourse);
    }
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
    json["color"] = color.name();
    json["star_associated"] = starAssociated;
    json["associated_course"] = associatedCourse;
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
    NodeShapes shape = Circle;
    if (json.contains("shape")) {
        QString shapeString = json["shape"].toString();
        shape = stringToShape(shapeString);
    }
    QColor color = Qt::cyan;
    if (json.contains("color")) {
        color = QColor(json["color"].toString());
    }
    Node *node = new Node(x, y, text, font, shape);
    node->setColor(color);
    if (json.contains("connections") && json["connections"].isArray()) {
        QJsonArray connectionsArray = json["connections"].toArray();
        for (const QJsonValue &value : connectionsArray) {
            if (value.isDouble()) {
                node->addConnection(value.toInt());
            }
        }
    }
    if (json.contains("star_associated")) {
        node->setStarAssociated(json["star_associated"].toBool());
    }
    if (json.contains("associated_course")) {
        QString associatedCourse = json["associated_course"].toString();
        node->setAssociatedCourse(associatedCourse);
        MainWindow::associatedCourses.append(associatedCourse);
    }
    node->adjustNodeSize();
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
    if (modified) {
        for (QGraphicsPixmapItem *item : starItems) {
            if (item && item->data(0).toString() == "StarItem") {
                return;
            }
        }
        QPixmap starPixmap = TextureCache::getTexture("resources/textures/star-collected.png");
        if (starPixmap.isNull()) {
            qDebug() << "Failed to load star image.";
            return;
        }

        QGraphicsPixmapItem *starItem = new QGraphicsPixmapItem(starPixmap, this);
        if (starItem) {
            starItem->setScale(1.0 / 2.0);
            QPointF starPos = boundingRect().topRight() - QPointF(starPixmap.width() / 3.0, 0);
            starItem->setPos(starPos);
            starItem->setData(0, "StarItem"); 
            starItems.append(starItem);
        } else {
            qDebug() << "Failed to create QGraphicsPixmapItem.";
        }
    }
}