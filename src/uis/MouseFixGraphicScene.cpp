#include "MouseFixGraphicScene.h"

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
void MouseFixGraphicScene::updateLineItemComputations() {
    QPointF mousePosScene = lastMousePos;
    qreal margin = 333.0;
    QRectF sceneRect = MainWindow::graphicsScene->sceneRect().adjusted(margin, margin, -margin, -margin);
    if (!sceneRect.contains(mousePosScene)) {
        return;
    }

    if (MainWindow::shiftPressed && MainWindow::startNodeIndex != -1) {
        Node *startNode = MainWindow::nodes[MainWindow::startNodeIndex];
        if (startNode) {
            QRectF boundingBox = startNode->boundingRect();
            boundingBox = startNode->mapRectToScene(boundingBox); // Map to scene coordinates

            bool mouseInBoundingBox = boundingBox.contains(mousePosScene);
            if (mouseInBoundingBox) {
                // Remove previous line item if needed
                if (previousLineItem && MainWindow::graphicsScene->items().contains(previousLineItem)) {
                    if (boundingBox.contains(mousePosScene)) {
                        MainWindow::graphicsScene->removeItem(previousLineItem);
                        previousLineItem = nullptr;
                    }
                }
                return;
            }

            // Draw the line as expected
            QPointF startEdgePoint = getNodeEdgePoint(startNode, mousePosScene);
            QLineF line(startEdgePoint, mousePosScene);

            // Save line details to update later in the GUI
            QMetaObject::invokeMethod(this, [this, line]() {
                if (previousLineItem) {
                    if (MainWindow::graphicsScene->items().contains(previousLineItem)) {
                        MainWindow::graphicsScene->removeItem(previousLineItem);
                    }
                    previousLineItem = nullptr;
                }
                previousLineItem = MainWindow::graphicsScene->addLine(line, QPen(Qt::DashLine)); }, Qt::QueuedConnection);
        }
    } else {
        // Remove previous line item if Shift key is not pressed
        QMetaObject::invokeMethod(this, [this]() {
            if (previousLineItem) {
                if (MainWindow::graphicsScene->items().contains(previousLineItem)) {
                    MainWindow::graphicsScene->removeItem(previousLineItem);
                }
                previousLineItem = nullptr;
            } }, Qt::QueuedConnection);
    }
}

void MouseFixGraphicScene::onUpdateLineItem() {
    updateFuture = QtConcurrent::run([this]() {
        updateLineItemComputations();
    });
}

MouseFixGraphicScene::MouseFixGraphicScene(QObject *parent)
    : QGraphicsScene(parent), updateTimer(new QTimer(this)) {
    connect(updateTimer, &QTimer::timeout, this, &MouseFixGraphicScene::onUpdateLineItem);
    updateTimer->start(100); // Update every 100 ms
}
void MouseFixGraphicScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    lastMousePos = event->scenePos();
    QGraphicsScene::mouseMoveEvent(event);
}

void MouseFixGraphicScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
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
    QGraphicsScene::mouseReleaseEvent(event);
}
void MouseFixGraphicScene::wheelEvent(QGraphicsSceneWheelEvent *event) {
    if (event->modifiers() & Qt::ShiftModifier) {
        QGraphicsView *view = views().at(0);
        qreal scaleFactor = 1.2;
        qreal scaleAmount = (event->delta() > 0) ? scaleFactor : 1.0 / scaleFactor;
        qreal minScale = 0.03;
        qreal maxScale = 10.0;
        qreal currentScale = view->transform().m11();
        qreal newScale = currentScale * scaleAmount;
        if (newScale < minScale) {
            scaleAmount = minScale / currentScale;
            newScale = minScale;
        } else if (newScale > maxScale) {
            scaleAmount = maxScale / currentScale;
            newScale = maxScale;
        }
        QPointF mousePosInScene = event->scenePos();
        view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        view->scale(scaleAmount, scaleAmount);
        view->setTransformationAnchor(QGraphicsView::NoAnchor);
        QPointF newMousePosInScene = view->mapToScene(view->mapFromScene(mousePosInScene));
        QPointF delta = newMousePosInScene - mousePosInScene;
        view->translate(delta.x(), delta.y());
        event->accept();
    } else {
        QGraphicsScene::wheelEvent(event);
    }
}