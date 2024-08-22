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
MainWindow::MainWindow() {
    setWindowTitle("Mind Map Example");
    setFixedSize(WIDTH, HEIGHT);
    // Initialisation des objets graphiques
    emulatorText = new QLabel("Emulator Status", this);
    b3313Text = new QLabel("B3313 V1.0.2 Status", this);
    // Initialisation de la vue graphique et de la scène
    graphicsView = new QGraphicsView(this);
    graphicsScene = new MouseFixGraphicScene(this);
    graphicsView->setScene(graphicsScene);
    // Initialisation du widget central et du layout
    centralWidgetZ = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidgetZ);
    setCentralWidget(centralWidgetZ);
    // Ajout de la vue graphique et des boutons au layout
    layout->addWidget(graphicsView);
    saveButton = new QPushButton("Save", this);
    switchViewButton = new QPushButton("Switch View", this);
    layout->addWidget(saveButton);
    layout->addWidget(switchViewButton);
    // Connexion des boutons aux slots correspondants
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveNodes);
    connect(switchViewButton, &QPushButton::clicked, this, &MainWindow::toggleStarDisplay);
    contextMenu = new QMenu(this);
    QAction *removeConnectionsAction = new QAction("Remove Connections", this);
    connect(removeConnectionsAction, &QAction::triggered, this, &MainWindow::removeConnections);
    contextMenu->addAction(removeConnectionsAction);
    // Initialisation des objets graphiques
    layout->addWidget(emulatorText);
    layout->addWidget(b3313Text);
    b3313Text->hide();
    emulatorText->hide();

    // Initialisation et démarrage du thread de mise à jour
    thread = std::make_unique<MainWindowUpdateThread>(this);
    connect(thread.get(), &MainWindowUpdateThread::updateNeeded, this, &MainWindow::onTimerUpdate);
    thread->start();
    // Configuration des propriétés de la scène
    QRectF sceneBoundingRect = graphicsScene->itemsBoundingRect();
    QRectF adjustedSceneRect = sceneBoundingRect.adjusted(0, 0, 50000, 50000); // Ajout d'une marge
    graphicsScene->setSceneRect(adjustedSceneRect);
    // Initialisation du widget de l'affichage des étoiles
    star_display_mainLayout = layout; // Use the existing layout

    // Chargement des données JSON
    loadJsonData(b33_13_mind_map_str);

    // Set focus policy
    setFocusPolicy(Qt::StrongFocus);
    graphicsView->setFocusPolicy(Qt::StrongFocus);
    // graphicsScene->setFocusPolicy(Qt::StrongFocus);
    // Install event filter
    installEventFilter(this);
    graphicsView->installEventFilter(this);
    // graphicsScene->installEventFilter(this);

    // Enable mouse tracking
    setMouseTracking(true);
    graphicsView->setMouseTracking(true);
    // graphicsScene->setMouseTracking(this);

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
        thread->stop(); // Demander au thread de s'arrêter
        thread->wait(); // Attendre la fin du thread
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
    if (event->key() == Qt::Key_Shift && !shiftPressed) {
        shiftPressed = true;
        setNodesMovable(false); // Désactive le déplacement des nœuds
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift && shiftPressed) {
        shiftPressed = false;
        setNodesMovable(true); // Réactive le déplacement des nœuds
        startNodeIndex = -1;
    }
}

void MainWindow::setNodesMovable(bool movable) {
    for (Node *node : nodes) {
        node->setMovable(movable); // Met à jour la propriété de mobilité des nœuds
    }
}
void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        QPointF mousePos = graphicsView->mapToScene(event->pos());
        int nodeIndex;
        if (isMouseOverNode(mousePos, nodeIndex)) {
            rightClickedNodeIndex = nodeIndex;
            contextMenu->exec(QCursor::pos());
        }
    }
    if (shiftPressed && event->button() == Qt::LeftButton) {
        startPos = graphicsView->mapToScene(event->pos());
        int nodeIndex;
        if (isMouseOverNode(startPos, nodeIndex)) 
            startNodeIndex = nodeIndex;
    }
#ifdef DEBUG
    if (!shiftPressed && event->button() == Qt::RightButton) {
        QPoint viewPos = event->pos();
        QPointF scenePos = graphicsView->mapToScene(viewPos);
        if (scenePos.x() < 0 || scenePos.y() < 0) {
            qDebug() << "Invalid scene coordinates, skipping node creation.";
            return;
        }
        Node *newNode = new Node(scenePos.x(), scenePos.y(), "New Node", font());
        QPointF nodePos = newNode->pos();
        QRectF nodeRect = newNode->boundingRect();
        newNode->setModified(true);
        graphicsScene->addItem(newNode);
        nodes.append(newNode);
    }
#endif
}

void MainWindow::removeConnections() {
    if (rightClickedNodeIndex != -1) {
        if (rightClickedNodeIndex >= 0 && rightClickedNodeIndex < nodes.size()) {
            Node *nodeToRemove = nodes[rightClickedNodeIndex];
            QVector<int> indicesToRemove;
            for (int i = 0; i < connections.size(); ++i) {
                QPair<int, int> conn = connections[i];
                if (conn.first == rightClickedNodeIndex || conn.second == rightClickedNodeIndex)
                    indicesToRemove.push_back(i);
            }
            for (int i = indicesToRemove.size() - 1; i >= 0; --i) {
                connections.removeAt(indicesToRemove[i]);
            }
            for (int i : nodeToRemove->getConnections()) {
                if (i >= 0 && i < nodes.size())
                    nodes[i]->removeConnection(rightClickedNodeIndex);
                else
                    qDebug() << "Invalid node index in removeConnections.";
            }
            nodeToRemove->connections.clear();
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

    // Ajouter les connexions en tant que tableau JSON
    QJsonObject connectionsJson;
    QJsonArray connectionsArray;
    for (const auto &connection : connections) {
        QJsonObject connectionObj;
        connectionObj["start"] = connection.first;
        connectionObj["end"] = connection.second;
        connectionsArray.append(connectionObj);
    }
    connectionsJson["connections"] = connectionsArray;

    // Ajouter les connexions au JSON principal
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

void MainWindow::printWidgetOrder() {
    QVBoxLayout *layout = star_display_mainLayout;
    qDebug() << "Current widget order in layout:";
    for (int i = 0; i < layout->count(); ++i) {
        QWidget *widget = layout->itemAt(i)->widget();
        if (widget) {
            qDebug() << "Widget:" << widget->objectName();
        }
    }
}

void MainWindow::toggleStarDisplay() {
    showStarDisplay = !showStarDisplay;
    if (showStarDisplay) {
        switchViewButton->hide();
        QRectF sceneBoundingRect = graphicsView->rect();
        graphicsScene->setSceneRect(sceneBoundingRect);
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
        QRectF sceneBoundingRect = graphicsScene->itemsBoundingRect();
        QRectF adjustedSceneRect = sceneBoundingRect.adjusted(0, 0, 50000, 50000);
        graphicsScene->setSceneRect(adjustedSceneRect);
        QVBoxLayout *layout = star_display_mainLayout;
        // Retirer saveButton s'il est présent
        if (layout->indexOf(saveButton) != -1)
            layout->removeWidget(saveButton);
        // Ajouter saveButton avant switchViewButton
        layout->addWidget(saveButton);
        // Retirer switchViewButton s'il est présent
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

QJsonObject MainWindow::loadJsonData2(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file:" << filePath;
        return QJsonObject(); // Return an empty QJsonObject on failure
    }
    QByteArray jsonData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isObject()) {
        qWarning() << "Invalid JSON format in file:" << filePath;
        return QJsonObject(); // Return an empty QJsonObject on failure
    }
    return jsonDoc.object(); // Return the parsed QJsonObject
}
void MainWindow::loadJsonData(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << filename;
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    if (doc.isNull() || !doc.isObject()) { // Vérifiez si c'est un QJsonObject
        qWarning() << "Failed to parse JSON or JSON is not an object.";
        return;
    }
    QJsonObject jsonObject = doc.object();
    lastJsonData = jsonObject; // Clear lastJsonData if not used
    parseJsonData(jsonObject);
}

void MainWindow::parseJsonData(const QJsonObject &jsonObj) {
    nodes.clear();
    connections.clear();
    graphicsScene->clear(); // Nettoyer la scène

    // Charger les nœuds
    if (jsonObj.contains("nodes") && jsonObj["nodes"].isArray()) {
        QJsonArray nodesArray = jsonObj["nodes"].toArray();
        QHash<int, Node *> nodeIndexMap;

        for (int i = 0; i < nodesArray.size(); ++i) {
            QJsonObject nodeObj = nodesArray[i].toObject();
            Node *node = Node::fromJson(nodeObj, QFont("Arial", 12, QFont::Bold));
            graphicsScene->addItem(node);
            nodes.append(node);
            nodeIndexMap.insert(i, node); // Utiliser l'index comme clé
        }

        // Charger les connexions
        if (jsonObj.contains("connections") && jsonObj["connections"].isObject()) {
            QJsonObject connectionsObj = jsonObj["connections"].toObject();
            if (connectionsObj.contains("connections") && connectionsObj["connections"].isArray()) {
                QJsonArray connectionsArray = connectionsObj["connections"].toArray();
                for (const QJsonValue &value : connectionsArray) {
                    QJsonObject connectionObj = value.toObject();
                    int startIndex = connectionObj["start"].toInt();
                    int endIndex = connectionObj["end"].toInt();

                    if (nodeIndexMap.contains(startIndex) && nodeIndexMap.contains(endIndex)) {
                        Node *startNode = nodeIndexMap[startIndex];
                        Node *endNode = nodeIndexMap[endIndex];

                        if (startNode && endNode) {
                            connections.push_back(QPair<int, int>(startIndex, endIndex));
                            nodes[startIndex]->addConnection(endIndex);
                            nodes[endIndex]->addConnection(startIndex);
                        }
                    } else {

                        qWarning() << "Invalid node index in connection.";
                    }
                }
            }
        }
    }
}

void MainWindow::onTimerUpdate() {
    updateDisplay();
}

void MainWindow::updateDisplay() {
    isModified();
    if (showStarDisplay) {
        textUpdate();
        QJsonObject jsonData = loadJsonData2("resources/stars_layout/b3313-V1.0.2/star_display_layout.json");
        starDisplay.displayStars(jsonData);
    } else {
        // Clear existing arrows before redrawing
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

        // Redraw the arrows
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

                // Ajoutez la flèche
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
    }
    isModified();
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
