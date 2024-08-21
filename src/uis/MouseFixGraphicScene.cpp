#include "MouseFixGraphicScene.h"

QGraphicsLineItem *previousLineItem = nullptr;

QPointF getNodeEdgePoint(const Node *node, const QPointF &endPoint) {
    QPointF nodeCenter = node->pos();
    QRectF nodeRect = node->boundingRect();
    QPointF nodeTopLeft = nodeCenter + QPointF(nodeRect.left(), nodeRect.top());
    QPointF nodeBottomRight = nodeCenter + QPointF(nodeRect.right(), nodeRect.bottom());
    QLineF line(nodeCenter, endPoint);
    QPointF edgePoint = line.p2();
    line.setLength(nodeRect.width() / 2);
    edgePoint = line.p2();
    return edgePoint;
}

void MouseFixGraphicScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseMoveEvent(event);
    QPointF mousePosScene = event->scenePos();
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
