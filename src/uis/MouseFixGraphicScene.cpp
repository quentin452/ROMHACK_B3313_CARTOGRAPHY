#include "MouseFixGraphicScene.h"

QGraphicsLineItem *previousLineItem = nullptr;
QPointF getNodeEdgePoint(const Node *node, const QPointF &endPoint) {
    QPointF nodeCenter = node->pos();
    QLineF line(nodeCenter, endPoint);
    QPointF direction = line.unitVector().p2() - nodeCenter;

    QPointF edgePoint;
    switch (node->shape) {
    case Circle: {
        float radiusX = node->boundingRect().width() / 2;  // Rayon horizontal de l'ellipse
        float radiusY = node->boundingRect().height() / 2; // Rayon vertical de l'ellipse
        // Définir les points de la ligne et la direction
        QPointF direction = line.p2() - nodeCenter;
        float length = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());
        if (length == 0) {
            edgePoint = nodeCenter; // Si le point de destination est au centre de l'ellipse
        } else {
            // Normaliser la direction
            QPointF unitDirection = direction / length;

            // Calculer les points d'intersection avec l'ellipse
            float t = std::sqrt((radiusX * radiusX * radiusY * radiusY) / (radiusY * radiusY * unitDirection.x() * unitDirection.x() + radiusX * radiusX * unitDirection.y() * unitDirection.y()));
            edgePoint = nodeCenter + unitDirection * t;

            qDebug() << "Ellipse Edge Point:" << edgePoint;
        }
        break;
    }
    case Square: {
        QRectF rect = node->boundingRect();
        QPointF topLeft = rect.topLeft() + nodeCenter;
        QPointF topRight = rect.topRight() + nodeCenter;
        QPointF bottomLeft = rect.bottomLeft() + nodeCenter;
        QPointF bottomRight = rect.bottomRight() + nodeCenter;

        QList<QPointF> points = {topLeft, topRight, bottomRight, bottomLeft};
        QLineF edges[4] = {
            QLineF(topLeft, topRight),
            QLineF(topRight, bottomRight),
            QLineF(bottomRight, bottomLeft),
            QLineF(bottomLeft, topLeft)};

        for (const QLineF &edge : edges) {
            QPointF intersection;
            if (line.intersects(edge, &intersection) == QLineF::BoundedIntersection) {
                edgePoint = intersection;
                break;
            }
        }
        break;
    }
    case Triangle: {
        QRectF rect = node->boundingRect();
        QPointF top = QPointF(rect.center().x(), rect.top()) + nodeCenter;
        QPointF bottomLeft = QPointF(rect.left(), rect.bottom()) + nodeCenter;
        QPointF bottomRight = QPointF(rect.right(), rect.bottom()) + nodeCenter;

        QList<QLineF> edges = {
            QLineF(top, bottomLeft),
            QLineF(bottomLeft, bottomRight),
            QLineF(bottomRight, top)};

        for (const QLineF &edge : edges) {
            QPointF intersection;
            if (line.intersects(edge, &intersection) == QLineF::BoundedIntersection) {
                edgePoint = intersection;
                break;
            }
        }
        break;
    }
    default:
        edgePoint = nodeCenter;
        break;
    }

    return edgePoint;
}

void MouseFixGraphicScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QPointF mousePosScene = event->scenePos();
    qreal margin = 333.0;
    QRectF sceneRect = MainWindow::graphicsScene->sceneRect().adjusted(margin, margin, -margin, -margin);
    if (!sceneRect.contains(mousePosScene))
        return;
    QGraphicsScene::mouseMoveEvent(event);
    if (MainWindow::shiftPressed && MainWindow::startNodeIndex != -1) {
        Node *startNode = MainWindow::nodes[MainWindow::startNodeIndex];
        if (startNode) {
            QPointF startEdgePoint = getNodeEdgePoint(startNode, mousePosScene);
            QLineF line(startEdgePoint, mousePosScene);
            if (previousLineItem) {
                if (MainWindow::graphicsScene->items().contains(previousLineItem))
                    MainWindow::graphicsScene->removeItem(previousLineItem);
                previousLineItem = nullptr;
            }
            previousLineItem = MainWindow::graphicsScene->addLine(line, QPen(Qt::DashLine));
        }
    }
}

void MouseFixGraphicScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);
    if (MainWindow::shiftPressed && event->button() == Qt::LeftButton) {
        QPointF mousePos = event->scenePos();
        int endNodeIndex;
        bool isNodeOver = MainWindow::isMouseOverNode(mousePos, endNodeIndex);
        if (isNodeOver && endNodeIndex != MainWindow::startNodeIndex) {
            if (MainWindow::startNodeIndex >= 0 && MainWindow::startNodeIndex < MainWindow::nodes.size() &&
                endNodeIndex >= 0 && endNodeIndex < MainWindow::nodes.size()) {
                bool connectionExists = false;
                for (const auto &connection : MainWindow::connections) {
                    if ((connection.first == MainWindow::startNodeIndex && connection.second == endNodeIndex) ||
                        (connection.first == endNodeIndex && connection.second == MainWindow::startNodeIndex)) {
                        connectionExists = true;
                        break;
                    }
                }
                if (!connectionExists) {
                    MainWindow::connections.push_back(QPair<int, int>(MainWindow::startNodeIndex, endNodeIndex));
                    MainWindow::nodes[MainWindow::startNodeIndex]->addConnection(endNodeIndex);
                    MainWindow::nodes[endNodeIndex]->addConnection(MainWindow::startNodeIndex);
                    MainWindow::nodes[MainWindow::startNodeIndex]->setModified(true);
                    MainWindow::nodes[endNodeIndex]->setModified(true);
                }
            }
            MainWindow::startNodeIndex = -1;
        }
    }
    if (previousLineItem) {
        if (MainWindow::graphicsScene->items().contains(previousLineItem))
            MainWindow::graphicsScene->removeItem(previousLineItem);
        previousLineItem = nullptr;
    }
}