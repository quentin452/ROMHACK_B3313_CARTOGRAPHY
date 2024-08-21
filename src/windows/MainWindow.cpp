#include "MainWindow.h"
#ifdef DEBUG
QString b33_13_mind_map_str = "b3313-v1.0.2-Mind_map.json";
#else
QString b33_13_mind_map_str = "stars_layout/b3313-V1.0.2/b3313-v1.0.2-Mind_map.json";
#endif

MainWindow::MainWindow() {
    setWindowTitle("Mind Map Example");
    setFixedSize(WIDTH, HEIGHT);

    // Initialisation des objets graphiques
    emulatorText = new QGraphicsTextItem("Emulator Status");
    b3313Text = new QGraphicsTextItem("B3313 V1.0.2 Status");
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
            mind_map_nodes = loadNodes(b33_13_mind_map_str, font);
        }
    });

    setMouseTracking(true);
    graphicsView->setMouseTracking(true);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
    QRectF sceneBoundingRect = graphicsScene->itemsBoundingRect();
    QRectF adjustedSceneRect = sceneBoundingRect.adjusted(0, 0, 50000, 50000); // Ajoutez une marge autour des nœuds
    graphicsScene->setSceneRect(adjustedSceneRect);
    loadJsonData(b33_13_mind_map_str);
    star_display_centralWidget = new QWidget(this);
    star_display_mainLayout = new QVBoxLayout(centralWidget);
    // Initialiser et démarrer le thread après toutes les autres initialisations
    thread = std::make_unique<MainWindowUpdateThread>(this);
    connect(thread.get(), &MainWindowUpdateThread::updateNeeded, this, &MainWindow::onTimerUpdate);
    thread->start();

    graphicsScene->addItem(emulatorText);
    graphicsScene->addItem(b3313Text);
    b3313Text->hide();
    emulatorText->hide();
    textUpdate();
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
        emulatorText->setPlainText(emulatorRunning ? "Emulator Running" : "Emulator Not Running");
        emulatorText->setDefaultTextColor(emulatorRunning ? Qt::green : Qt::black);
    } else {
        qWarning() << "emulatorText is null!";
    }

    if (b3313Text) {
        b3313Text->setPlainText(romLoaded ? "B3313 V1.0.2 ROM Loaded" : "B3313 V1.0.2 ROM Not Loaded");
        b3313Text->setDefaultTextColor(romLoaded ? Qt::green : Qt::black);
    } else {
        qWarning() << "b3313Text is null!";
    }
}
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift && !shiftPressed) {
        shiftPressed = true;
        dragging = false;
        setNodesMovable(false); // Désactive le déplacement des nœuds
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift && shiftPressed) {
        shiftPressed = false;
        setNodesMovable(true); // Réactive le déplacement des nœuds
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
        qDebug() << "Right Button Clicked at Scene Position:" << mousePos; // Afficher la position de la souris
        int nodeIndex;
        if (isMouseOverNode(mousePos, nodeIndex)) {
            rightClickedNodeIndex = nodeIndex;
            contextMenu->exec(QCursor::pos());
        }
    }
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
    if (!shiftPressed && event->button() == Qt::RightButton) {
        QPoint viewPos = event->pos();

        // Obtenir les coordonnées de la scène en tenant compte des transformations de la vue
        QPointF scenePos = graphicsView->mapToScene(viewPos);

        // Vérifiez que les coordonnées de la scène sont valides
        if (scenePos.x() < 0 || scenePos.y() < 0) {
            qDebug() << "Invalid scene coordinates, skipping node creation.";
            return;
        }

        // Crée un nouveau nœud à la position de la souris
        Node *newNode = new Node(scenePos.x(), scenePos.y(), "New Node", font());

        // Vérifiez la position du nœud après sa création
        QPointF nodePos = newNode->pos();

        // Vérifiez les dimensions et la position du nœud
        QRectF nodeRect = newNode->boundingRect();

        // Ajoutez le nœud à la scène
        newNode->setModified(true);
        graphicsScene->addItem(newNode);
        nodes.append(newNode);
    }
#endif
}
void MainWindow::mouseMoveEvent(QMouseEvent *event) {
    if (shiftPressed && dragging && startNodeIndex != -1) {
        QPointF mousePos = graphicsView->mapToScene(event->pos());
        Node *startNode = nodes[startNodeIndex];
        QLineF line(startNode->pos(), mousePos);
        currentArrow = new QGraphicsLineItem(line);
        currentArrow->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap));
        graphicsScene->addItem(currentArrow);
    }
}
void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
    if (shiftPressed && event->button() == Qt::LeftButton && dragging) {
        QPointF mousePos = graphicsView->mapToScene(event->pos());
        int endNodeIndex;
        if (isMouseOverNode(mousePos, endNodeIndex) && endNodeIndex != startNodeIndex) {
            qDebug() << "End Node Index:" << endNodeIndex;
            if (startNodeIndex >= 0 && startNodeIndex < nodes.size() &&
                endNodeIndex >= 0 && endNodeIndex < nodes.size()) {
                connections.push_back(QPair<int, int>(startNodeIndex, endNodeIndex));
                nodes[startNodeIndex]->addConnection(endNodeIndex);
                nodes[endNodeIndex]->addConnection(startNodeIndex);
                updateDisplay();
            } else {
                qDebug() << "Invalid node index in connections.";
            }
        } else {
            qDebug() << "No valid end node detected.";
        }
        dragging = false;
    }
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
    }
    QJsonDocument jsonDoc(jsonArray);
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
        QRectF sceneBoundingRect = graphicsView->rect();
        graphicsScene->setSceneRect(sceneBoundingRect);
        if (emulatorText) {

            if (!emulatorText->isVisible())
                emulatorText->show();
        }
        if (b3313Text) {
            if (!b3313Text->isVisible())
                b3313Text->show();
        }
        for (Node *node : nodes) {
            if (node) {
                node->hide();
            }
        }
    } else {
        QRectF sceneBoundingRect = graphicsScene->itemsBoundingRect();
        QRectF adjustedSceneRect = sceneBoundingRect.adjusted(0, 0, 50000, 50000);
        graphicsScene->setSceneRect(adjustedSceneRect);
        b3313Text->hide();
        emulatorText->hide();
        for (Node *node : nodes) {
            if (node) {
                node->show();
            }
        }
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
        if (node->isModified())
            return true;
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
    if (doc.isNull() || !doc.isArray()) {
        qWarning() << "Failed to parse JSON or JSON is not an array.";
        return;
    }
    QJsonArray jsonArray = doc.array();
    lastJsonData = QJsonObject(); // Clear lastJsonData if not used
    parseJsonData(jsonArray);
}

void MainWindow::parseJsonData(const QJsonArray &jsonArray) {
    nodes.clear();
    connections.clear();
    QFont defaultFont("Arial", 12, QFont::Bold); // Default font settings

    for (const QJsonValue &value : jsonArray) {
        if (!value.isObject()) {
            qWarning() << "Invalid node format.";
            continue;
        }
        QJsonObject nodeObj = value.toObject();
        if (!nodeObj.contains("x") || !nodeObj["x"].isDouble() ||
            !nodeObj.contains("y") || !nodeObj["y"].isDouble() ||
            !nodeObj.contains("text") || !nodeObj["text"].isString()) {
            qWarning() << "Node data is incomplete or invalid.";
            continue;
        }

        qreal x = nodeObj["x"].toDouble();
        qreal y = nodeObj["y"].toDouble();
        QString label = nodeObj["text"].toString();

        // Get font size from JSON, default to defaultFont size if not present
        int fontSize = defaultFont.pointSize(); // Default size
        if (nodeObj.contains("font_size") && nodeObj["font_size"].isDouble()) {
            fontSize = nodeObj["font_size"].toInt();
        }

        QFont font = defaultFont;
        font.setPointSize(fontSize);

        Node *node = new Node(x, y, label, font);
        graphicsScene->addItem(node);
        nodes.append(node);
    }

    if (jsonArray.size() > 1) {                         // Ensure jsonArray contains the connections
        QJsonObject jsonData = jsonArray[1].toObject(); // Assuming connections are in the second element
        QJsonArray connectionArray = jsonData["connections"].toArray();
        for (const QJsonValue &value : connectionArray) {
            QJsonObject connObj = value.toObject();
            int startIndex = connObj["start"].toInt();
            int endIndex = connObj["end"].toInt();
            if (startIndex >= 0 && startIndex < nodes.size() &&
                endIndex >= 0 && endIndex < nodes.size()) {
                connections.push_back(QPair<int, int>(startIndex, endIndex));
            } else {
                qDebug() << "Invalid connection indices in JSON data.";
            }
        }
    }
}
void MainWindow::onTimerUpdate() {
    updateDisplay();
}

void MainWindow::updateDisplay() {
    // Add nodes and texts
    if (showStarDisplay) {
        textUpdate();
        QJsonObject jsonData = loadJsonData2("resources/stars_layout/b3313-V1.0.2/star_display_layout.json");
        displayStars(jsonData);
    } else {
        // Draw connections
        for (const QPair<int, int> &conn : connections) {
            if (conn.first >= 0 && conn.first < nodes.size() &&
                conn.second >= 0 && conn.second < nodes.size()) {
                Node *startNode = nodes[conn.first];
                Node *endNode = nodes[conn.second];
                if (!startNode || !endNode) {
                    qWarning() << "Invalid node pointers for connection:" << conn;
                    continue;
                }
                qDebug() << "Drawing line from node" << conn.first << "to node" << conn.second;
                QGraphicsLineItem *lineItem = new QGraphicsLineItem(startNode->x(), startNode->y(), endNode->x(), endNode->y());
                lineItem->setPen(QPen(Qt::black));
                graphicsScene->addItem(lineItem);
            } else {
                qWarning() << "Connection has invalid node index:" << conn;
            }
        }
    }
}
void MainWindow::displayStars(const QJsonObject &jsonData) {
    if (isRomHackLoaded(global_detected_emulator)) {
        std::cerr << "RomHack is loaded." << std::endl;
        emulatorText->hide();
        b3313Text->hide();
        std::string saveLocation = GetParallelLauncherSaveLocation();
        std::cerr << "Save location: " << saveLocation << std::endl;

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
        std::cerr << "Save parameters parsed." << std::endl;

        auto saveData = ReadSrmFile(saveLocation, params);
        if (saveData.empty()) {
            std::cerr << "Erreur: Les données de sauvegarde sont vides." << std::endl;
            return;
        }
        std::cerr << "Save data read successfully." << std::endl;

        int numSlots = params.numSlots;
        if (numSlots <= 0) {
            std::cerr << "Erreur: Nombre de slots invalide." << std::endl;
            return;
        }
        std::cerr << "Number of slots: " << numSlots << std::endl;

        tabNames.clear();
        for (int i = 1; i <= numSlots; ++i) {
            tabNames.append("Mario " + QString::number(i));
        }

        QTabWidget *tabWidget = new QTabWidget(this);
        int yOffset = 0;
        int reservedHeight = 0; // Ajouté pour stocker la hauteur réservée

        for (int i = 0; i < numSlots; ++i) {
            QString tabName = tabNames[i];
            QWidget *tabContent = new QWidget();
            QVBoxLayout *layout = new QVBoxLayout(tabContent);

            QLabel *tabLabel = new QLabel(tabName, tabContent);
            QFont font = this->font();
            tabLabel->setFont(font);
            tabLabel->setStyleSheet("color: black;");
            layout->addWidget(tabLabel);

            for (const auto &groupValue : jsonData["groups"].toArray()) {
                QJsonObject group = groupValue.toObject();
                if (!group.contains("name") || !group.contains("courses")) {
                    std::cerr << "Erreur: Le groupe dans JSON est mal formé." << std::endl;
                    continue;
                }
                QString groupName = group["name"].toString();

                QMap<QString, QVector<StarData>> courseStarsMap;

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
                pixmap.fill(Qt::transparent);
                QPainter painter(&pixmap);
                painter.setRenderHint(QPainter::Antialiasing);
                starDisplay.afficherEtoilesGroupeFusionne(groupName, courseStarsMap, painter, font, yOffset, reservedHeight, graphicsView->rect());
                painter.end();
                QLabel *pixmapLabel = new QLabel(tabContent);
                pixmapLabel->setPixmap(pixmap);
                layout->addWidget(pixmapLabel);
            }
            tabContent->setLayout(layout);
            tabWidget->addTab(tabContent, tabName);
        }
        QWidget *currentCentralWidget = this->findChild<QWidget *>("centralWidget");
        if (currentCentralWidget != star_display_centralWidget) {
            star_display_mainLayout->addWidget(tabWidget);
            star_display_centralWidget->setLayout(star_display_mainLayout);
            setCentralWidget(star_display_centralWidget);
        }
    } else {
        emulatorText->show();
        b3313Text->show();
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
    if (!file.open(QIODevice::ReadOnly))
        return nodes;
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
