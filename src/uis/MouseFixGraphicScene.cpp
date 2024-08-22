#include "MouseFixGraphicScene.h"

QGraphicsLineItem *previousLineItem = nullptr;
QPointF getNodeEdgePoint(const Node *node, const QPointF &endPoint) {
    QPointF nodeCenter = node->pos();
    QLineF line(nodeCenter, endPoint);
    QPointF direction = line.unitVector().p2() - nodeCenter;

    QPointF edgePoint;
    switch (node->shape) {
    case Circle: {
        float radius = node->boundingRect().width() / 2; // Rayon du cercle

        // Définir les points de la ligne et la direction
        QPointF direction = line.p2() - nodeCenter;
        float length = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());

        if (length == 0) {
            edgePoint = nodeCenter; // Si le point de destination est au centre du cercle
        } else {
            // Normaliser la direction
            QPointF unitDirection = direction / length;

            // Équation de la ligne en termes de paramètres t
            // x = x0 + t * dx
            // y = y0 + t * dy
            // Substituer dans l'équation du cercle :
            // (x0 + t * dx - xc)^2 + (y0 + t * dy - yc)^2 = r^2
            // Développer pour obtenir une équation quadratique en t

            float dx = direction.x();
            float dy = direction.y();
            float A = dx * dx + dy * dy;
            float B = 2 * (dx * (nodeCenter.x() - line.p1().x()) + dy * (nodeCenter.y() - line.p1().y()));
            float C = (line.p1().x() - nodeCenter.x()) * (line.p1().x() - nodeCenter.x()) + (line.p1().y() - nodeCenter.y()) * (line.p1().y() - nodeCenter.y()) - radius * radius;

            // Résoudre l'équation quadratique At^2 + Bt + C = 0
            float discriminant = B * B - 4 * A * C;
            if (discriminant >= 0) {
                float sqrtDisc = std::sqrt(discriminant);
                float t1 = (-B - sqrtDisc) / (2 * A);
                float t2 = (-B + sqrtDisc) / (2 * A);

                // Calculer les points d'intersection
                QPointF intersection1 = line.p1() + unitDirection * t1;
                QPointF intersection2 = line.p1() + unitDirection * t2;

                // Choisir le point d'intersection le plus proche de endPoint
                float dist1 = QLineF(endPoint, intersection1).length();
                float dist2 = QLineF(endPoint, intersection2).length();

                if (dist1 < dist2) {
                    edgePoint = intersection1;
                } else {
                    edgePoint = intersection2;
                }

                qDebug() << "Circle Edge Point:" << edgePoint;
            } else {
                qDebug() << "Error: No intersection found.";
            }
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