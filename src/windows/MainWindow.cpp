#include "MainWindow.h"

MainWindow::MainWindow() {
    loadJsonData("resources/stars_layout/b3313-V1.0.2/layout.json");
    setWindowTitle("Mind Map Example");
    setFixedSize(WIDTH, HEIGHT);
    emulatorText = new QGraphicsTextItem("Emulator Status");
    b3313Text = new QGraphicsTextItem("B3313 V1.0.2 Status");
    tabManager = new TabManager(tabNames, this);
    graphicsView = new QGraphicsView(this);
    graphicsScene = new QGraphicsScene(this);
    graphicsView->setScene(graphicsScene);
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->addWidget(graphicsView);
    saveButton = new QPushButton("Save", this);
    switchViewButton = new QPushButton("Switch View", this);
    layout->addWidget(saveButton);
    layout->addWidget(switchViewButton);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveNodes);
    connect(switchViewButton, &QPushButton::clicked, this, &MainWindow::toggleStarDisplay);
    dropdownMenu = new QComboBox(this);
    dropdownMenu->addItems({"b3313-v1.0.2.json"});
    layout->addWidget(dropdownMenu);
    contextMenu = new QMenu(this);
    QAction *removeConnectionsAction = new QAction("Remove Connections", this);
    connect(removeConnectionsAction, &QAction::triggered, this, &MainWindow::removeConnections);
    contextMenu->addAction(removeConnectionsAction);
    connect(dropdownMenu, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index) {
        QString selectedFile = dropdownMenu->itemText(index);
        if (!selectedFile.isEmpty()) {
            QFont font = this->font();
            mind_map_nodes = loadNodes("b3313-v1.0.2.json", font);
        }
    });
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::onTimerUpdate);
    updateTimer->start(1000);
    setMouseTracking(true);
    graphicsView->setMouseTracking(true);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift && !shiftPressed) {
        shiftPressed = true;
        dragging = false;
        setNodesMovable(false);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift && shiftPressed) {
        shiftPressed = false;
        setNodesMovable(true);
    }
}

void MainWindow::setNodesMovable(bool movable) {
    for (Node *node : nodes) {
        node->setMovable(movable);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    // Gestion du clic droit pour afficher le menu contextuel
    if (event->button() == Qt::RightButton) {
        QPointF mousePos = graphicsView->mapToScene(event->pos());
        qDebug() << "Right Button Clicked at Scene Position:" << mousePos; // Afficher la position de la souris
        int nodeIndex;
        if (isMouseOverNode(mousePos, nodeIndex)) {
            rightClickedNodeIndex = nodeIndex;
            contextMenu->exec(QCursor::pos());
        }
    }

    // Gestion du clic gauche avec Shift pour créer une connexion entre les nœuds
    if (shiftPressed && event->button() == Qt::LeftButton) {
        startPos = graphicsView->mapToScene(event->pos());
        qDebug() << "Shift + Left Button Clicked at Scene Position:" << startPos; // Afficher la position de la souris
        int nodeIndex;
        if (isMouseOverNode(startPos, nodeIndex)) {
            startNodeIndex = nodeIndex;
            dragging = true;
            qDebug() << "Start Node Index:" << startNodeIndex; // Débogage
        }
    }

#ifdef DEBUG
    // Création d'un nouveau nœud si on clique avec le bouton droit sans Shift
    if (!shiftPressed && event->button() == Qt::RightButton) {
        QPointF scenePos = graphicsView->mapToScene(event->pos());
        qDebug() << "Creating New Node at Scene Position:" << scenePos; // Afficher la position de la souris

        // Utiliser un raycast pour détecter les objets sous la souris
        QGraphicsItem *itemUnderMouse = graphicsScene->itemAt(scenePos, QTransform());
        if (!itemUnderMouse) {
            // Crée un nouveau nœud à la position de la souris
            Node *newNode = new Node(scenePos.x(), scenePos.y(), "New Node", font());
            newNode->setModified(true);
            graphicsScene->addItem(newNode);
            nodes.append(newNode);

            // Débogage pour vérifier la position du nouveau nœud
            qDebug() << "New Node Created at Scene Position:" << newNode->pos();
        }
    }

#endif
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
}
void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (shiftPressed && event->button() == Qt::LeftButton && dragging) {
        QPointF mousePos = graphicsView->mapToScene(event->pos());
        int endNodeIndex;
        if (isMouseOverNode(mousePos, endNodeIndex) && endNodeIndex != startNodeIndex) {
            qDebug() << "End Node Index:" << endNodeIndex;
            connections.push_back(QPair<int, int>(startNodeIndex, endNodeIndex));
            nodes[startNodeIndex]->addConnection(endNodeIndex);
            nodes[endNodeIndex]->addConnection(startNodeIndex);
            updateDisplay(lastJsonData);
        } else {
            qDebug() << "No valid end node detected.";
        }
        dragging = false;
    }
}

void MainWindow::removeConnections() {
    if (rightClickedNodeIndex != -1) {
        Node *nodeToRemove = nodes[rightClickedNodeIndex];
        QVector<int> indicesToRemove;
        for (int i = 0; i < connections.size(); ++i) {
            QPair<int, int> conn = connections[i];
            if (conn.first == rightClickedNodeIndex || conn.second == rightClickedNodeIndex) {
                indicesToRemove.push_back(i);
            }
        }
        for (int i = indicesToRemove.size() - 1; i >= 0; --i) {
            connections.removeAt(indicesToRemove[i]);
        }
        // Retirer les connexions des autres nœuds
        for (int i : nodeToRemove->getConnections()) {
            nodes[i]->removeConnection(rightClickedNodeIndex);
        }
        nodeToRemove->connections.clear();
        updateDisplay(lastJsonData);
    }
}

void MainWindow::saveNodes() {
    QJsonArray jsonArray;
    for (const auto &nodePtr : mind_map_nodes) {
        jsonArray.append(nodePtr->toJson());
    }

    QJsonDocument jsonDoc(jsonArray);
    QFile file("b3313-v1.0.2.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(jsonDoc.toJson(QJsonDocument::Indented)); // Pretty print JSON
        file.close();
    }
}

void MainWindow::toggleStarDisplay() {
    showStarDisplay = !showStarDisplay;
    updateDisplay(lastJsonData);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (isModified()) {
        QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Mind Map Example",
                                                                   tr("You have unsaved changes.\nDo you want to save your changes before exiting?"),
                                                                   QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                   QMessageBox::Yes);
        if (resBtn == QMessageBox::Yes) {
            saveNodes();
            event->accept();
        } else if (resBtn == QMessageBox::No) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

bool MainWindow::isModified() const {
    for (const auto &node : nodes) {
        if (node->isModified()) {
            return true;
        }
    }
    return false;
}
void MainWindow::onTimerUpdate() {
    static QElapsedTimer elapsedTimer;
    static bool timerStarted = false;
    if (!timerStarted) {
        elapsedTimer.start();
        timerStarted = true;
    }
    qint64 elapsedMilliseconds = elapsedTimer.elapsed();
    if (elapsedMilliseconds < 1000) {
        return;
    }
    elapsedTimer.restart();
    bool emulatorRunning = isEmulatorDetected(parallelLauncher, global_detected_emulator);
    bool romLoaded = isRomHackLoaded(global_detected_emulator);

    emulatorText->setPlainText(emulatorRunning ? "Emulator Running" : "Emulator Not Running");
    emulatorText->setDefaultTextColor(emulatorRunning ? Qt::green : Qt::black);

    b3313Text->setPlainText(romLoaded ? "B3313 V1.0.2 ROM Loaded" : "B3313 V1.0.2 ROM Not Loaded");
    b3313Text->setDefaultTextColor(romLoaded ? Qt::green : Qt::black);
    updateDisplay(lastJsonData);
}
void MainWindow::loadJsonData(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << filename;
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument doc(QJsonDocument::fromJson(data));
    QJsonObject jsonData = doc.object();
    lastJsonData = jsonData; // Store the loaded JSON data
    parseJsonData(jsonData);
}

void MainWindow::parseJsonData(const QJsonObject &jsonData) {
    nodes.clear();
    connections.clear();
    QFont defaultFont("Arial", 12, QFont::Bold);
    QJsonArray nodeArray = jsonData["nodes"].toArray();
    for (const QJsonValue &value : nodeArray) {
        QJsonObject nodeObj = value.toObject();
        qreal x = nodeObj["x"].toDouble();
        qreal y = nodeObj["y"].toDouble();
        QString label = nodeObj["label"].toString();
        Node *node = new Node(x, y, label, defaultFont);
        graphicsScene->addItem(node);
        nodes.append(node);
    }
    QJsonArray connectionArray = jsonData["connections"].toArray();
    for (const QJsonValue &value : connectionArray) {
        QJsonObject connObj = value.toObject();
        int startIndex = connObj["start"].toInt();
        int endIndex = connObj["end"].toInt();
        if (startIndex >= 0 && startIndex < nodes.size() && endIndex >= 0 && endIndex < nodes.size()) {
            connections.push_back(QPair<int, int>(startIndex, endIndex));
        }
    }
}
void MainWindow::updateDisplay(const QJsonObject &jsonData) {
    static QElapsedTimer elapsedTimer;
    static bool timerStarted = false;
    if (!timerStarted) {
        elapsedTimer.start();
        timerStarted = true;
    }
    qint64 elapsedMilliseconds = elapsedTimer.elapsed();
    if (elapsedMilliseconds < 1000) {
        return; // Exit early if not enough time has passed
    }
    // Update timer
    elapsedTimer.restart();
    graphicsScene->clear();

    // Check emulator status and update text color
    bool emulatorRunning = isEmulatorDetected(parallelLauncher, global_detected_emulator);
    bool romLoaded = isRomHackLoaded(global_detected_emulator);
    emulatorText->setPlainText(emulatorRunning ? "Emulator Running" : "Emulator Not Running");
    emulatorText->setDefaultTextColor(emulatorRunning ? Qt::green : Qt::black);
    b3313Text->setPlainText(romLoaded ? "B3313 V1.0.2 ROM Loaded" : "B3313 V1.0.2 ROM Not Loaded");
    b3313Text->setDefaultTextColor(romLoaded ? Qt::green : Qt::black);
    for (const QPair<int, int> &conn : connections) {
        Node *startNode = nodes[conn.first];
        Node *endNode = nodes[conn.second];
        QGraphicsLineItem *lineItem = new QGraphicsLineItem(startNode->x(), startNode->y(), endNode->x(), endNode->y());
        lineItem->setPen(QPen(Qt::black));
        graphicsScene->addItem(lineItem);
    }
    if (showStarDisplay) {
        displayStars(jsonData);
    } else {
        for (const QPair<int, int> &conn : connections) {
            Node *startNode = nodes[conn.first];
            Node *endNode = nodes[conn.second];
            QGraphicsLineItem *lineItem = new QGraphicsLineItem(startNode->x(), startNode->y(), endNode->x(), endNode->y());
            lineItem->setPen(QPen(Qt::black));
            graphicsScene->addItem(lineItem);
        }
        for (Node *node : nodes) {
            node->updateStar(); // Met à jour l'étoile pour chaque nœud
            graphicsScene->addItem(node);
        }
        if (emulatorText) {
            graphicsScene->addItem(emulatorText);
        }
        if (b3313Text) {
            graphicsScene->addItem(b3313Text);
        }
    }
}

void MainWindow::displayStars(const QJsonObject &jsonData) {
    if (isRomHackLoaded(global_detected_emulator)) {
        std::string saveLocation = GetParallelLauncherSaveLocation();
        if (!jsonData.contains("format") || !jsonData["format"].toObject().contains("save_type") ||
            !jsonData["format"].toObject().contains("slots_start") || !jsonData["format"].toObject().contains("slot_size") ||
            !jsonData["format"].toObject().contains("active_bit") || !jsonData["format"].toObject().contains("checksum_offset")) {
            std::cerr << "Erreur: Les paramètres de sauvegarde sont manquants dans le JSON." << std::endl;
            return;
        }
        QJsonObject format = jsonData["format"].toObject();
        SaveParams params;
        params.saveFormat = parseSaveFormat(format["save_type"].toString().toStdString());
        params.slotsStart = format["slots_start"].toInt();
        params.slotSize = format["slot_size"].toInt();
        params.activeBit = format["active_bit"].toInt();
        params.numSlots = format["num_slots"].toInt();
        params.checksumOffset = format["checksum_offset"].toInt();
        auto saveData = ReadSrmFile(saveLocation, params);
        if (saveData.empty()) {
            std::cerr << "Erreur: Les données de sauvegarde sont vides." << std::endl;
            return;
        }
        int yOffset = 0;
        int numSlots = params.numSlots;
        if (numSlots <= 0) {
            std::cerr << "Erreur: Nombre de slots invalide." << std::endl;
            return;
        }
        tabNames.clear();
        for (int i = 1; i <= numSlots; ++i) {
            tabNames.append("Mario " + QString::number(i));
        }
        tabManager->initializeTabs(tabNames);
        QString currentTabName = tabManager->getCurrentTabName();
        if (currentTabName.isEmpty()) {
            std::cerr << "Erreur: Nom de l'onglet actuel est vide." << std::endl;
            return;
        }
        float reservedHeight = tabManager->getTabsHeight();
        QRectF windowRect = graphicsView->rect();
        QPainter painter;
        for (int i = 0; i < numSlots; ++i) {
            if (i >= tabNames.size()) {
                std::cerr << "Erreur: Index de tabName hors limites." << std::endl;
                continue;
            }
            QString tabName = tabNames[i];
            if (tabName == currentTabName) {
                QGraphicsTextItem *tabText = new QGraphicsTextItem(tabName);
                QFont font = this->font();
                tabText->setFont(font);
                tabText->setDefaultTextColor(Qt::black);
                tabText->setPos(100, 100 + yOffset);
                graphicsScene->addItem(tabText);
                yOffset += 30;
                for (const auto &groupValue : jsonData["groups"].toArray()) {
                    QJsonObject group = groupValue.toObject();
                    if (!group.contains("name") || !group.contains("courses")) {
                        std::cerr << "Erreur: Le groupe dans JSON est mal formé." << std::endl;
                        continue;
                    }
                    QString groupName = group["name"].toString();
                    QMap<QString, QVector<StarData>> courseStarsMap;
                    yOffset += 30;
                    for (const auto &courseValue : group["courses"].toArray()) {
                        QJsonObject course = courseValue.toObject();
                        if (!course.contains("name") || !course.contains("data")) {
                            std::cerr << "Erreur: Le cours dans JSON est mal formé." << std::endl;
                            continue;
                        }
                        QString courseName = course["name"].toString();
                        QVector<StarData> &courseStarList = courseStarsMap[courseName];
                        for (const auto &dataValue : course["data"].toArray()) {
                            QJsonObject data = dataValue.toObject();
                            int offset = data["offset"].toInt();
                            int mask = data["mask"].toInt();
                            int numStars = 1;
                            for (int bit = 0; bit < 32; ++bit) {
                                if (mask & (1 << bit)) {
                                    bool star_collected = isStarCollected(saveData, offset, bit, i, params.slotSize);
                                    courseStarList.append({courseName, numStars, star_collected, offset, mask});
                                }
                            }
                        }
                    }
                    QPixmap pixmap(graphicsView->size());
                    pixmap.fill(Qt::white);
                    painter.begin(&pixmap);
                    starDisplay.afficherEtoilesGroupeFusionne(groupName, courseStarsMap, painter, font, yOffset, reservedHeight, windowRect);
                    painter.end();
                    graphicsScene->addPixmap(pixmap);
                }
                graphicsView->setScene(graphicsScene);
                break;
            }
        }
    } else {
        graphicsScene->addItem(emulatorText);
        graphicsScene->addItem(b3313Text);
    }
}

bool MainWindow::isMouseOverNode(const QPointF &mousePos, int &nodeIndex) {
    for (int i = 0; i < nodes.size(); ++i) {
        Node *node = nodes[i];
        if (node->contains(mousePos)) {
            nodeIndex = i;
            return true;
        }
    }
    return false;
}
QVector<Node *> MainWindow::loadNodes(const QString &filename, QFont &font) {
    QVector<Node *> nodes;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return nodes;
    }
    QByteArray fileData = file.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(fileData));
    QJsonArray jsonArray = jsonDoc.array();
    for (const QJsonValue &value : jsonArray) {
        if (value.isObject()) {
            QJsonObject jsonObject = value.toObject();
            Node *node = new Node(Node::fromJson(jsonObject, font)); // Create a new Node object
            nodes.push_back(node);                                   // Add pointer to vector
        }
    }

    return nodes;
}
#include "MainWindow.moc"
