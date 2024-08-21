#include "MouseFixGraphicScene.h"

void MouseFixGraphicScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseMoveEvent(event);

    // Les coordonnées fournies par l'événement sont déjà en coordonnées de la scène
    QPointF mousePosScene = event->scenePos();

    if (MainWindow::shiftPressed && MainWindow::startNodeIndex != -1) {
        Node *startNode = MainWindow::nodes[MainWindow::startNodeIndex];

        if (startNode) {
            QLineF line(startNode->pos(), mousePosScene);

            if (MainWindow::preDrawLineItem) {
                MainWindow::graphicsScene->removeItem(MainWindow::preDrawLineItem);
                delete MainWindow::preDrawLineItem;
                MainWindow::preDrawLineItem = nullptr;
            }

            MainWindow::preDrawLineItem = MainWindow::graphicsScene->addLine(line, QPen(Qt::DashLine));
        }

        QApplication::setOverrideCursor(Qt::CrossCursor);
    }
}

void MouseFixGraphicScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);

    if (MainWindow::shiftPressed && event->button() == Qt::LeftButton) {
        QPointF mousePos = event->scenePos();
        qDebug() << "Mouse released at scene position:" << mousePos;

        int endNodeIndex;
        bool isNodeOver = isMouseOverNode(mousePos, endNodeIndex);

        qDebug() << "Node over check result:" << isNodeOver << ", End Node Index:" << endNodeIndex;

        if (isNodeOver && endNodeIndex != MainWindow::startNodeIndex) {
            if (MainWindow::startNodeIndex >= 0 && MainWindow::startNodeIndex < MainWindow::nodes.size() &&
                endNodeIndex >= 0 && endNodeIndex < MainWindow::nodes.size()) {

                qDebug() << "Adding connection between nodes:" << MainWindow::startNodeIndex << "and" << endNodeIndex;
                MainWindow::connections.push_back(QPair<int, int>(MainWindow::startNodeIndex, endNodeIndex));
                MainWindow::nodes[MainWindow::startNodeIndex]->addConnection(endNodeIndex);
                MainWindow::nodes[endNodeIndex]->addConnection(MainWindow::startNodeIndex);

                MainWindow::addConnectionToScene(MainWindow::startNodeIndex, endNodeIndex);
            }
        }

        if (MainWindow::preDrawLineItem) {
            MainWindow::graphicsScene->removeItem(MainWindow::preDrawLineItem);
            delete MainWindow::preDrawLineItem;
            MainWindow::preDrawLineItem = nullptr;
        }

        QApplication::restoreOverrideCursor();
    }
}

bool MouseFixGraphicScene::isMouseOverNode(const QPointF &mousePosScene, int &nodeIndex) {
    qDebug() << "Checking mouse position in scene coordinates:" << mousePosScene;

    for (int i = 0; i < MainWindow::nodes.size(); ++i) {
        Node *node = MainWindow::nodes[i];
        QPointF mousePosLocal = node->mapFromScene(mousePosScene);
        QRectF nodeRect = node->boundingRect();

        qDebug() << "Checking node at position:" << node->pos();
        qDebug() << "Node bounding rect:" << nodeRect;
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
