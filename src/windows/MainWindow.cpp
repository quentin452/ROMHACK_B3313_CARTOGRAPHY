#include "MainWindow.h"
#ifdef DEBUG
QString b33_13_mind_map_str = "b3313-v1.0.2-Mind_map.json";
#else
QString b33_13_mind_map_str = "stars_layout/b3313-V1.0.2/b3313-v1.0.2-Mind_map.json";
#endif
std::basic_string<wchar_t> MainWindow::global_detected_emulator;
QLabel *MainWindow::emulatorText, *MainWindow::b3313Text = nullptr;
QStringList MainWindow::tabNames;
QGraphicsView *MainWindow::graphicsView = nullptr;
QTabWidget *MainWindow::tabWidget = nullptr;
QFont MainWindow::qfont;
QVBoxLayout *MainWindow::star_display_mainLayout = nullptr;
QPushButton *MainWindow::switchViewButton = nullptr;
QVector<Node *> MainWindow::nodes;
bool MainWindow::shiftPressed = false;
int MainWindow::startNodeIndex = -1;
QVector<QPair<int, int>> MainWindow::connections;
QGraphicsScene *MainWindow::graphicsScene = nullptr;
QJsonObject MainWindow::lastJsonData;

MainWindow::MainWindow() {
    setWindowTitle("Mind Map Example");
    setFixedSize(WIDTH, HEIGHT);
    emulatorText = new QLabel("Emulator Status", this);
    b3313Text = new QLabel("B3313 V1.0.2 Status", this);
    graphicsView = new QGraphicsView(this);
    graphicsScene = new MouseFixGraphicScene(this);
    graphicsView->setScene(graphicsScene);
    centralWidgetZ = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidgetZ);
    setCentralWidget(centralWidgetZ);
    layout->addWidget(graphicsView);
    saveButton = new QPushButton("Save", this);
    switchViewButton = new QPushButton("Switch View", this);
    layout->addWidget(saveButton);
    layout->addWidget(switchViewButton);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveNodes);
    connect(switchViewButton, &QPushButton::clicked, this, &MainWindow::toggleStarDisplay);
    contextMenu = new QMenu(this);
    QAction *removeConnectionsAction = new QAction("Remove Connections", this);
    connect(removeConnectionsAction, &QAction::triggered, this, &MainWindow::removeConnections);
    contextMenu->addAction(removeConnectionsAction);
    layout->addWidget(emulatorText);
    layout->addWidget(b3313Text);
    b3313Text->hide();
    emulatorText->hide();
    thread = std::make_unique<MainWindowUpdateThread>(this);
    connect(thread.get(), &MainWindowUpdateThread::updateNeeded, this, &MainWindow::onTimerUpdate);
    thread->start();
    QRectF sceneBoundingRect = graphicsScene->itemsBoundingRect();
    QRectF adjustedSceneRect = sceneBoundingRect.adjusted(0, 0, 50000, 50000);
    graphicsScene->setSceneRect(adjustedSceneRect);
    star_display_mainLayout = layout;
    JsonLoading::loadNodesJsonData(b33_13_mind_map_str);
    setFocusPolicy(Qt::StrongFocus);
    graphicsView->setFocusPolicy(Qt::StrongFocus);
    installEventFilter(this);
    graphicsView->installEventFilter(this);
    setMouseTracking(true);
    graphicsView->setMouseTracking(true);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    textUpdate();
    tabWidget = findChild<QTabWidget *>("tabWidget");
    if (!tabWidget) {
        tabWidget = new QTabWidget(this);
        tabWidget->setObjectName("tabWidget");
        star_display_mainLayout->insertWidget(0, tabWidget);
    }
    star_display_mainLayout->insertWidget(5, switchViewButton);
    tabWidget->hide();
    qfont = this->font();
}

MainWindow::~MainWindow() {
    if (thread) {
        thread->stop();
        thread->wait();
    }
}
void MainWindow::textUpdate() {
    bool emulatorRunning = isEmulatorDetected(parallelLauncher, global_detected_emulator);
    bool romLoaded = isRomHackLoaded(global_detected_emulator);

    if (emulatorText) {
        emulatorText->setText(emulatorRunning ? "Parallel launcher Emulator Running" : "Parallel launcher Emulator Not Running");
        emulatorText->setStyleSheet(emulatorRunning ? "color: green;" : "color: white;");
    } else {
        qWarning() << "emulatorLabel is null!";
    }

    if (b3313Text) {
        b3313Text->setText(romLoaded ? "B3313 V1.0.2 ROM Loaded" : "B3313 V1.0.2 ROM Not Loaded");
        b3313Text->setStyleSheet(romLoaded ? "color: green;" : "color: white;");
    } else {
        qWarning() << "b3313Label is null!";
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        shiftPressed = true;
        setNodesMovable(false);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        shiftPressed = false;
        setNodesMovable(true);
        startNodeIndex = -1;
    }
}

void MainWindow::setNodesMovable(bool movable) {
    for (Node *node : nodes) {
        node->setMovable(movable);
    }
}
void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (shiftPressed && event->button() == Qt::RightButton) {
        startNodeIndex = -1;
        QPointF mousePos = graphicsView->mapToScene(event->pos());
        int nodeIndex;
        if (isMouseOverNode(mousePos, nodeIndex)) {
            rightClickedNodeIndex = nodeIndex;
            contextMenuOpened = true;
            contextMenu->exec(QCursor::pos());
            shiftPressed = false;
        }
    }

    if (!contextMenuOpened && event->button() == Qt::LeftButton) {
        startPos = graphicsView->mapToScene(event->pos());
        int nodeIndex;
        if (isMouseOverNode(startPos, nodeIndex))
            startNodeIndex = nodeIndex;
    }
#ifdef DEBUG
    if (!contextMenuOpened && !shiftPressed && event->button() == Qt::RightButton) {
        QPointF scenePos = graphicsView->mapToScene(event->pos());

        int nodeIndex;
        if (isMouseOverNode(scenePos, nodeIndex)) {
            return;
        }

        if (scenePos.x() < 0 || scenePos.y() < 0) {
            qDebug() << "Invalid scene coordinates, skipping node creation.";
            return;
        }

        Node *newNode = new Node(scenePos.x(), scenePos.y(), "New Node", font());
        newNode->setModified(true);
        graphicsScene->addItem(newNode);
        nodes.append(newNode);
    }
#endif
    if (contextMenuOpened)
        contextMenuOpened = false;
}

void MainWindow::removeConnections() {
    if (rightClickedNodeIndex != -1) {
        if (rightClickedNodeIndex >= 0 && rightClickedNodeIndex < nodes.size()) {
            Node *nodeToRemove = nodes[rightClickedNodeIndex];
            QVector<int> indicesToRemove, connectedNodes;
            for (int i = 0; i < connections.size(); ++i) {
                QPair<int, int> conn = connections[i];
                if (conn.first == rightClickedNodeIndex || conn.second == rightClickedNodeIndex) {
                    indicesToRemove.push_back(i);
                    int connectedNodeIndex = (conn.first == rightClickedNodeIndex) ? conn.second : conn.first;
                    if (connectedNodeIndex >= 0 && connectedNodeIndex < nodes.size()) {
                        connectedNodes.push_back(connectedNodeIndex);
                    }
                }
            }
            for (int i = indicesToRemove.size() - 1; i >= 0; --i) {
                connections.removeAt(indicesToRemove[i]);
            }
            for (int i : connectedNodes) {
                if (i >= 0 && i < nodes.size()) {
                    nodes[i]->removeConnection(rightClickedNodeIndex);
                    nodes[i]->setModified(true);
                } else {
                    qDebug() << "Invalid node index in removeConnections.";
                }
            }
            nodeToRemove->connections.clear();
            nodeToRemove->setModified(true);
            updateDisplay();
        } else {
            qDebug() << "Invalid node index in removeConnections.";
        }
    }
}

void MainWindow::saveNodes() {
    QJsonArray jsonArray;
    for (const auto &nodePtr : nodes) {
        jsonArray.append(nodePtr->toJson());
        nodePtr->setModified(false);
    }
    QJsonObject connectionsJson;
    QJsonArray connectionsArray;
    for (const auto &connection : connections) {
        QJsonObject connectionObj;
        connectionObj["start"] = connection.first;
        connectionObj["end"] = connection.second;
        connectionsArray.append(connectionObj);
    }
    connectionsJson["connections"] = connectionsArray;
    QJsonObject mainJson;
    mainJson["nodes"] = jsonArray;
    mainJson["connections"] = connectionsJson;
    QJsonDocument jsonDoc(mainJson);
    QFile file(b33_13_mind_map_str);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(jsonDoc.toJson(QJsonDocument::Indented)); // Pretty print JSON
        file.close();
    } else {
        qWarning() << "Failed to open file for writing:" << file.errorString();
    }
}

void MainWindow::toggleStarDisplay() {
    showStarDisplay = !showStarDisplay;
    if (showStarDisplay) {
        switchViewButton->hide();
        QTabWidget *tabWidget = findChild<QTabWidget *>("tabWidget");
        if (tabWidget)
            tabWidget->hide();
        graphicsView->hide();
        saveButton->hide();
        if (emulatorText)
            emulatorText->show();
        if (b3313Text)
            b3313Text->show();
        for (Node *node : nodes) {
            if (node)
                node->hide();
        }
        b3313Text->hide();
        emulatorText->hide();
    } else {
        QTabWidget *tabWidget = findChild<QTabWidget *>("tabWidget");
        if (tabWidget)
            tabWidget->hide();
        if (tabWidget) {
            while (tabWidget->count() > 0) {
                QWidget *tabContent = tabWidget->widget(0);
                tabWidget->removeTab(0);
                delete tabContent;
            }
        }
        graphicsView->show();
        b3313Text->hide();
        emulatorText->hide();
        for (Node *node : nodes) {
            if (node)
                node->show();
        }
        QVBoxLayout *layout = star_display_mainLayout;
        if (layout->indexOf(saveButton) != -1)
            layout->removeWidget(saveButton);
        layout->addWidget(saveButton);
        if (layout->indexOf(switchViewButton) != -1)
            layout->removeWidget(switchViewButton);
        layout->addWidget(switchViewButton);
        saveButton->show();
        switchViewButton->show();
        emulatorText->hide();
        b3313Text->hide();
    }
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (isModified()) {
        auto resBtn = QMessageBox::question(this, "Mind Map Example",
                                            tr("You have unsaved changes.\nDo you want to save your changes before exiting?"),
                                            QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                            QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes)
            saveNodes();
        if (resBtn != QMessageBox::Cancel)
            event->accept();
        else
            event->ignore();
    } else {
        event->accept();
    }
}

bool MainWindow::isModified() const {
    for (const auto &node : nodes) {
        if (node->isModified()) {
            node->updateIsModified();
            return true;
        }
    }
    return false;
}

void MainWindow::onTimerUpdate() {
    updateDisplay();
}

void MainWindow::updateDisplay() {
    if (showStarDisplay) {
        textUpdate();
        QJsonObject jsonData = JsonLoading::loadJsonData2("resources/stars_layout/b3313-V1.0.2/star_display_layout.json");
        starDisplay.displayStars(jsonData);
    } else {
        QList<QGraphicsItem *> items = graphicsScene->items();
        for (QGraphicsItem *item : items) {
            if (QGraphicsLineItem *lineItem = dynamic_cast<QGraphicsLineItem *>(item)) {
                graphicsScene->removeItem(lineItem);
                delete lineItem;
            } else if (QGraphicsPolygonItem *polygonItem = dynamic_cast<QGraphicsPolygonItem *>(item)) {
                graphicsScene->removeItem(polygonItem);
                delete polygonItem;
            }
        }
        for (const QPair<int, int> &conn : connections) {
            if (conn.first >= 0 && conn.first < nodes.size() &&
                conn.second >= 0 && conn.second < nodes.size()) {
                Node *startNode = nodes[conn.first];
                Node *endNode = nodes[conn.second];
                if (!startNode || !endNode) {
                    qWarning() << "Invalid node pointers for connection:" << conn;
                    continue;
                }
                QPointF startEdgePoint = getNodeEdgePoint(startNode, endNode->pos());
                QPointF endEdgePoint = getNodeEdgePoint(endNode, startNode->pos());
                QGraphicsLineItem *lineItem = new QGraphicsLineItem(startEdgePoint.x(), startEdgePoint.y(), endEdgePoint.x(), endEdgePoint.y());
                lineItem->setPen(QPen(Qt::black));
                graphicsScene->addItem(lineItem);
                QLineF line(startEdgePoint, endEdgePoint);
                double angle = std::atan2(-line.dy(), line.dx());
                QPointF arrowP1 = line.p2() - QPointF(std::sin(angle + M_PI / 3) * 10, std::cos(angle + M_PI / 3) * 10);
                QPointF arrowP2 = line.p2() - QPointF(std::sin(angle + M_PI - M_PI / 3) * 10, std::cos(angle + M_PI - M_PI / 3) * 10);
                QPolygonF arrowHead;
                arrowHead << line.p2() << arrowP1 << arrowP2;
                QGraphicsPolygonItem *arrowItem = new QGraphicsPolygonItem(arrowHead);
                arrowItem->setBrush(Qt::black);
                graphicsScene->addItem(arrowItem);
            } else {
                qWarning() << "Connection has invalid node index:" << conn;
            }
        }
        isModified();
    }
}

bool MainWindow::isMouseOverNode(const QPointF &mousePos, int &nodeIndex) {
    for (int i = 0; i < nodes.size(); ++i) {
        Node *node = nodes[i];
        QPointF mousePosLocal = node->mapFromScene(mousePos);
        QRectF nodeRect = node->boundingRect();
        if (nodeRect.contains(mousePosLocal)) {
            nodeIndex = i;
            return true;
        }
    }
    return false;
}
#include "MainWindow.moc"
