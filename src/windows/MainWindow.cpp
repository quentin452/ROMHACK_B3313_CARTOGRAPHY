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
    // Initialisation de la vue graphique et de la scène
    graphicsView = new QGraphicsView(this);
    graphicsScene = new QGraphicsScene(this);
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
    graphicsScene->addItem(emulatorText);
    graphicsScene->addItem(b3313Text);
    b3313Text->hide();
    emulatorText->hide();
    // Initialisation et démarrage du thread de mise à jour
    thread = std::make_unique<MainWindowUpdateThread>(this);
    connect(thread.get(), &MainWindowUpdateThread::updateNeeded, this, &MainWindow::onTimerUpdate);
    thread->start();
    // Chargement des données JSON
    loadJsonData(b33_13_mind_map_str);
    // Configuration des propriétés de la scène
    QRectF sceneBoundingRect = graphicsScene->itemsBoundingRect();
    QRectF adjustedSceneRect = sceneBoundingRect.adjusted(0, 0, 50000, 50000); // Ajout d'une marge
    graphicsScene->setSceneRect(adjustedSceneRect);
    // Initialisation du widget de l'affichage des étoiles
    star_display_mainLayout = layout; // Use the existing layout
    setMouseTracking(true);
    graphicsView->setMouseTracking(true);
    layout->setSpacing(10);
    layout->setContentsMargins(10, 10, 10, 10);
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
        int nodeIndex;
        if (isMouseOverNode(mousePos, nodeIndex)) {
            rightClickedNodeIndex = nodeIndex;
            contextMenu->exec(QCursor::pos());
        }
    }
    if (shiftPressed && event->button() == Qt::LeftButton) {
        startPos = graphicsView->mapToScene(event->pos());
        int nodeIndex;
        if (isMouseOverNode(startPos, nodeIndex)) {
            startNodeIndex = nodeIndex;
            dragging = true;
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
        QTabWidget *tabWidget = findChild<QTabWidget *>("tabWidget");
        if (tabWidget) 
            tabWidget->show();
        // Masquer les éléments de la vue graphique principale
        graphicsView->hide();
        saveButton->hide();
        // Afficher les textes et les éléments du layout des étoiles
        QRectF sceneBoundingRect = graphicsView->rect();
        graphicsScene->setSceneRect(sceneBoundingRect);
        if (emulatorText)
            emulatorText->show();
        if (b3313Text)
            b3313Text->show();
        for (Node *node : nodes) {
            if (node) {
                node->hide();
            }
        }
        // Appel pour afficher les étoiles
        QJsonObject jsonData = loadJsonData2("resources/stars_layout/b3313-V1.0.2/star_display_layout.json");
        displayStars(jsonData);
    } else {
        // Masquez le tabWidget s'il existe
        QTabWidget *tabWidget = findChild<QTabWidget *>("tabWidget");
        if (tabWidget)
            tabWidget->hide();
        // Vous pouvez aussi supprimer tous les onglets si nécessaire
        if (tabWidget) {
            while (tabWidget->count() > 0) {
                QWidget *tabContent = tabWidget->widget(0);
                tabWidget->removeTab(0);
                delete tabContent;
            }
        }
        // Réafficher les éléments de la vue graphique principale
        graphicsView->show();
        // Masquer les textes et les éléments du layout des étoiles
        b3313Text->hide();
        emulatorText->hide();
        for (Node *node : nodes) {
            if (node) {
                node->show();
            }
        }
        QRectF sceneBoundingRect = graphicsScene->itemsBoundingRect();
        QRectF adjustedSceneRect = sceneBoundingRect.adjusted(0, 0, 50000, 50000);
        graphicsScene->setSceneRect(adjustedSceneRect);
        QVBoxLayout *layout = star_display_mainLayout;
        // Retirer les boutons de leur position actuelle
        layout->removeWidget(switchViewButton);
        layout->removeWidget(saveButton);
        // Ajouter saveButton avant switchViewButton
        layout->addWidget(saveButton);
        layout->addWidget(switchViewButton);
        saveButton->show();
        switchViewButton->show();
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
        if (nodeObj.contains("font_size") && nodeObj["font_size"].isDouble()) 
            fontSize = nodeObj["font_size"].toInt();
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
    if (showStarDisplay) {
        textUpdate();
        QJsonObject jsonData = loadJsonData2("resources/stars_layout/b3313-V1.0.2/star_display_layout.json");
        displayStars(jsonData);
    } else {
        for (const QPair<int, int> &conn : connections) {
            if (conn.first >= 0 && conn.first < nodes.size() &&
                conn.second >= 0 && conn.second < nodes.size()) {
                Node *startNode = nodes[conn.first];
                Node *endNode = nodes[conn.second];
                if (!startNode || !endNode) {
                    qWarning() << "Invalid node pointers for connection:" << conn;
                    continue;
                }
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
        QTabWidget *tabWidget = findChild<QTabWidget *>("tabWidget");
        if (!tabWidget) {
            tabWidget = new QTabWidget(this);
            tabWidget->setObjectName("tabWidget"); // Donnez un nom au widget pour le retrouver plus tard
            if (star_display_mainLayout->indexOf(tabWidget) == -1) 
                star_display_mainLayout->addWidget(tabWidget);
        }
        emulatorText->hide();
        b3313Text->hide();
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
        int numSlots = params.numSlots;
        if (numSlots <= 0) {
            std::cerr << "Erreur: Nombre de slots invalide." << std::endl;
            return;
        }
        tabNames.clear();
        for (int i = 1; i <= numSlots; ++i) {
            tabNames.append("Mario " + QString::number(i));
        }
        int yOffset = reservedHeight = 0;
        for (int i = 0; i < numSlots; ++i) {
            QString tabName = tabNames[i];
            QWidget *tabContent = nullptr;
            if (tabWidget->count() > i) {
                tabContent = tabWidget->widget(i);
                QLayoutItem *child;
                while ((child = tabContent->layout()->takeAt(0)) != nullptr) {
                    delete child->widget();
                    delete child;
                }
            } else {
                tabContent = new QWidget();
                tabWidget->addTab(tabContent, tabName);
            }
            QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(tabContent->layout());
            if (!layout) {
                layout = new QVBoxLayout(tabContent);
                tabContent->setLayout(layout);
            }
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
#include "MainWindow.moc"
