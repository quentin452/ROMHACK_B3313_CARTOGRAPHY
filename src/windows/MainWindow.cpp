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

MainWindow::MainWindow() {
    setWindowTitle("Mind Map Example");
    setFixedSize(WIDTH, HEIGHT);
    // Initialisation des objets graphiques
    emulatorText = new QLabel("Emulator Status", this);
    b3313Text = new QLabel("B3313 V1.0.2 Status", this);
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
    layout->addWidget(emulatorText);
    layout->addWidget(b3313Text);
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
        if (layout->indexOf(saveButton) != -1) {
            layout->removeWidget(saveButton);
        }
        // Ajouter saveButton avant switchViewButton
        layout->addWidget(saveButton);
        // Retirer switchViewButton s'il est présent
        if (layout->indexOf(switchViewButton) != -1) {
            layout->removeWidget(switchViewButton);
        }
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
        starDisplay.displayStars(jsonData);
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
