#include "MouseFixGraphicScene.h"

void MouseFixGraphicScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseMoveEvent(event);
    if (MainWindow::shiftPressed && MainWindow::startNodeIndex != -1) {
        QPointF mousePos = MainWindow::graphicsView->mapToScene(event->pos().toPoint());
        Node *startNode = MainWindow::nodes[MainWindow::startNodeIndex];

        qDebug() << "Shift key is pressed and a valid start node index is set.";
        qDebug() << "Mouse Position in Scene Coordinates:" << mousePos;

        if (startNode) {
            qDebug() << "Start Node Position:" << startNode->pos();
        } else {
            qWarning() << "Start node pointer is null!";
        }

        QLineF line(startNode->pos(), mousePos);

        qDebug() << "Drawing line from start node to mouse position.";

        QApplication::setOverrideCursor(Qt::CrossCursor);
        qDebug() << "Cursor set to CrossCursor.";
    }
}

void MouseFixGraphicScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);
    if (MainWindow::shiftPressed && event->button() == Qt::LeftButton) {
        QPointF mousePos = MainWindow::graphicsView->mapToScene(event->pos().toPoint());
        qDebug() << "Mouse Position in Scene Coordinates:" << mousePos;

        int endNodeIndex;
        bool isNodeOver = isMouseOverNode(mousePos, endNodeIndex);
        qDebug() << "isMouseOverNode result:" << isNodeOver;

        if (isNodeOver && endNodeIndex != MainWindow::startNodeIndex) {
            qDebug() << "End Node Index:" << endNodeIndex;

            if (MainWindow::startNodeIndex >= 0 && MainWindow::startNodeIndex < MainWindow::nodes.size() &&
                endNodeIndex >= 0 && endNodeIndex < MainWindow::nodes.size()) {
                qDebug() << "Start Node Index:" << MainWindow::startNodeIndex;
                qDebug() << "Number of Nodes:" << MainWindow::nodes.size();

                MainWindow::connections.push_back(QPair<int, int>(MainWindow::startNodeIndex, endNodeIndex));
                MainWindow::nodes[MainWindow::startNodeIndex]->addConnection(endNodeIndex);
                MainWindow::nodes[endNodeIndex]->addConnection(MainWindow::startNodeIndex);

                qDebug() << "Connections:" << MainWindow::connections;

                MainWindow::addConnectionToScene(MainWindow::startNodeIndex, endNodeIndex); // Ajoute la connexion à la scène
            } else {
                qDebug() << "Invalid node index in connections. Start Node Index:" << MainWindow::startNodeIndex << "End Node Index:" << endNodeIndex;
            }
        } else {
            qDebug() << "No valid end node detected. End Node Index:" << endNodeIndex << "Start Node Index:" << MainWindow::startNodeIndex;
        }
    }
    QApplication::restoreOverrideCursor();
}

bool MouseFixGraphicScene::isMouseOverNode(const QPointF &mousePosScene, int &nodeIndex) {
    qDebug() << "Checking mouse position:" << mousePosScene;

    for (int i = 0; i < MainWindow::nodes.size(); ++i) {
        Node *node = MainWindow::nodes[i];

        // Vérifiez la transformation appliquée au nœud
        QTransform transform = node->transform();
        QPointF mousePosLocal = transform.inverted().map(mousePosScene);

        QRectF nodeRect = node->boundingRect();
        QPointF nodePos = node->pos();
        QRectF nodeRectInScene = nodeRect.translated(nodePos);

        qDebug() << "Checking node at position:" << nodePos;
        qDebug() << "Node bounding rect:" << nodeRect;
        qDebug() << "Node rect in scene:" << nodeRectInScene;
        qDebug() << "Mouse position in node's local coordinates:" << mousePosLocal;

        if (nodeRect.contains(mousePosLocal)) {
            nodeIndex = i;
            qDebug() << "Mouse is over node at index:" << i;
            return true;
        }
    }
    qDebug() << "Mouse is not over any node.";
    return false;
}
