#include "MainWindow.h"

std::basic_string<wchar_t> MainWindow::global_detected_emulator;
QLabel *MainWindow::emulatorText, *MainWindow::b3313Text = nullptr;
QStringList MainWindow::tabNames;
QGraphicsView *MainWindow::graphicsView = nullptr;
QTabWidget *MainWindow::tabWidget = nullptr;
QFont MainWindow::qfont;
QVBoxLayout *MainWindow::star_display_mainLayout = nullptr;
QPushButton *MainWindow::switchViewButton = nullptr, *MainWindow::settingsButton = nullptr;
QMap<QString, QRectF> MainWindow::courseNameRects, MainWindow::logoRects;
QVector<Node *> MainWindow::nodes;
bool MainWindow::shiftPressed = false, MainWindow::showStarDisplay = false, MainWindow::force_toggle_star_display = false, MainWindow::jump_to_star_display_associated_line = false;
int MainWindow::startNodeIndex = -1;
QVector<QPair<int, int>> MainWindow::connections;
QGraphicsScene *MainWindow::graphicsScene = nullptr;
QJsonObject MainWindow::lastJsonData;
QStringList MainWindow::courseNames, MainWindow::associatedCourses;
QString MainWindow::jump_to_which_line;
QScrollArea *MainWindow::scrollArea = nullptr;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      settingsWindow(nullptr) {
    setWindowTitle("Mind Map Example");
    setFixedSize(WIDTH, HEIGHT);
    emulatorText = new QLabel("Emulator Status", this);
    b3313Text = new QLabel("B3313 V1.0.2 Status", this);
    graphicsView = new CustomGraphicView(this);
    graphicsScene = new CustomGraphicScene(this);
    graphicsView->setScene(graphicsScene);
    centralWidgetZ = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidgetZ);
    setCentralWidget(centralWidgetZ);
    saveButton = new QPushButton("Save", this);
    switchViewButton = new QPushButton("Switch View", this);
    addWidgets(*layout, emulatorText, b3313Text, graphicsView, saveButton, switchViewButton);
    HIDE_WIDGETS(emulatorText, b3313Text);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveNodes);
    connect(switchViewButton, &QPushButton::clicked, this, &MainWindow::toggleStarDisplay);
    contextMenu = new QMenu(this);
    ADD_ACTION(contextMenu, renameNodeAction, renameSelectedNode)
    ADD_ACTION(contextMenu, removeConnectionsAction, removeConnections)
    ADD_ACTION(contextMenu, changeShapeAction, changeNodeShape)
    ADD_ACTION(contextMenu, changeColorAction, changeNodeColor)
    ADD_ACTION(contextMenu, associateStarAction, associateStarToNode)
    main_window_thread = std::make_unique<MainWindowUpdateThread>(this);
    connect(main_window_thread.get(), &MainWindowUpdateThread::updateNeeded, this, &MainWindow::onTimerUpdate);
    main_window_thread->start();
    QRectF sceneBoundingRect = graphicsScene->itemsBoundingRect();
    QRectF adjustedSceneRect = sceneBoundingRect.adjusted(0, 0, 50000, 50000);
    graphicsScene->setSceneRect(adjustedSceneRect);
    QPointF sceneCenter = adjustedSceneRect.center();
    graphicsView->centerOn(sceneCenter);
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
    tabWidget = findChild<QTabWidget *>("tabWidget");
    if (!tabWidget) {
        tabWidget = new QTabWidget(this);
        tabWidget->setObjectName("tabWidget");
        star_display_mainLayout->insertWidget(0, tabWidget);
    }
    star_display_mainLayout->insertWidget(5, switchViewButton);
    HIDE_WIDGETS(tabWidget);
    qfont = this->font();
    settingsWindow = new SettingsWindow(this);
    setWindowResizable(settingsWindow->isResizable());
    settingsButton = new QPushButton("Settings", this);
    star_display_mainLayout->addWidget(settingsButton);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::openSettingsWindow);
    QJsonObject jsonData = JsonLoading::loadJsonData2(GLOBAL_STAR_DISPLAY_JSON_STR);
    courseNames = getCourseNamesFromSlot0(jsonData);
    jsonLoaderThread = new JsonLoaderThread(this);
    connect(jsonLoaderThread, &JsonLoaderThread::jsonLoaded, this, &MainWindow::initializeStarDisplay);
    connect(jsonLoaderThread, &JsonLoaderThread::finished, jsonLoaderThread, &QObject::deleteLater);
    jsonLoaderThread->start();
    setupMinimap();
}
void MainWindow::setupMinimap() {
    return;
    // Initialiser la minimap view et scene
    miniMapScene = new QGraphicsScene(this);
    miniMapView = new MiniMapView(miniMapScene, this);

    // Positionner la minimap en haut à droite de la fenêtre principale
    miniMapView->setGeometry(QRect(width() - 200, 0, 200, 200)); // Ajuster la taille et la position selon les besoins

    // Configurer la vue de la minimap
    miniMapView->setRenderHint(QPainter::Antialiasing);
    miniMapView->setRenderHint(QPainter::SmoothPixmapTransform);
    miniMapView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    miniMapView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Connecter le clic de la minimap à la fonction de téléportation
    connect(miniMapView, &MiniMapView::minimapClicked, this, &MainWindow::onMinimapClick);
    syncMinimapView();
}
void MainWindow::onMinimapClick(QMouseEvent *event) {
    return;
    QPointF minimapClickPos = miniMapView->mapToScene(event->pos());

    // Calculer la position correspondante dans la scène principale
    QRectF sceneRect = miniMapScene->sceneRect();
    QRectF viewRect = graphicsView->scene()->sceneRect();

    // Convertir la position cliquée sur la minimap en position dans la scène principale
    QPointF mainScenePos = viewRect.topLeft() + (minimapClickPos - sceneRect.topLeft()) / miniMapView->transform().m11();

    // Déplacer la vue principale vers la nouvelle position
    graphicsView->centerOn(mainScenePos);
}
void MainWindow::updateMinimap() {
    return;
    // Assurer que la minimap est mise à jour pour refléter la scène principale
    miniMapScene->clear(); // Effacer l'ancienne vue de la minimap

    // Créer un aperçu de la scène principale
    QPixmap minimapPixmap(graphicsView->scene()->sceneRect().size().toSize());
    QPainter painter(&minimapPixmap);
    graphicsView->scene()->render(&painter);

    // Ajouter l'aperçu à la scène de la minimap
    miniMapScene->addPixmap(minimapPixmap);
}
void MainWindow::syncMinimapView() {
    return;
    // Synchroniser la vue principale avec la minimap
    QRectF mainViewRect = graphicsView->viewport()->rect();
    QRectF miniMapRect = miniMapView->viewport()->rect();
    miniMapView->fitInView(graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
    miniMapView->ensureVisible(mainViewRect, 0, 0);
}
void MainWindow::setWindowResizable(bool resizable) {
    if (resizable) {
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    } else {
        setFixedSize(WIDTH, HEIGHT);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
}
MainWindow::~MainWindow() {
    STOP_AND_WAIT_THREAD(main_window_thread);
    STOP_AND_WAIT_THREAD(jsonLoaderThread);
}

void MainWindow::openSettingsWindow() {
    settingsWindow->exec();
}
void MainWindow::textUpdate() {
    UPDATE_LABEL(emulatorText, isEmulatorDetected(parallelLauncher, global_detected_emulator), "Parallel launcher Emulator Running", "Parallel launcher Emulator Not Running");
    UPDATE_LABEL(b3313Text, isRomHackLoaded(global_detected_emulator), "B3313 V1.0.2 ROM Loaded", "B3313 V1.0.2 ROM Not Loaded");
}
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        shiftPressed = true;
        REPA(Node, nodes, setMovable(false))
    }
    if (event->key() == Qt::Key_Control) {
        REPA(Node, nodes, setMovable(false))
    }
    if (event->key() == Qt::Key_S && event->modifiers() & Qt::ControlModifier)
        saveNodes();
    if (event->key() == Qt::Key_F11) {
        QGraphicsView *view = findChild<QGraphicsView *>();
        QPointF centerPos = view->mapToScene(view->viewport()->rect().center());
        if (isFullScreen())
            showNormal();
        else
            showFullScreen();
        view->centerOn(centerPos);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        shiftPressed = false;
        REPA(Node, nodes, setMovable(true))
        startNodeIndex = -1;
    }
    if (event->key() == Qt::Key_Control) {
        REPA(Node, nodes, setMovable(true))
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    if (showStarDisplay) {
        QPointF pos = event->pos();
        QPointF adjustedPos = pos - scrollArea->widget()->pos() - scrollArea->widget()->mapToParent(QPoint(0, 0));
        QString courseClicked;
        QRectF clickedRect;
        for (auto it = courseNameRects.cbegin(); it != courseNameRects.cend(); ++it) {
            QRectF rect = it.value();
            if (rect.contains(adjustedPos)) {
                courseClicked = it.key();
                clickedRect = rect;
                break;
            }
        }
        if (courseClicked.isEmpty()) {
            for (auto it = logoRects.cbegin(); it != logoRects.cend(); ++it) {
                QRectF rect = it.value();
                if (rect.contains(adjustedPos)) {
                    courseClicked = it.key();
                    break;
                }
            }
        }
        if (!courseClicked.isEmpty()) { // THIS IS NEVER REACHED
            for (Node *node : nodes) {
                if (node->getAssociatedCourse() == courseClicked) {
                    force_toggle_star_display = true;
                    graphicsView->centerOn(node->pos());
                    break;
                }
            }
        }
    } else {
        QPointF mousePosScene = graphicsView->mapToScene(event->pos());
        qreal margin = 333.0;
        QRectF sceneRect = graphicsScene->sceneRect().adjusted(margin, margin, -margin, -margin);
        if (!sceneRect.contains(mousePosScene))
            return;
        if (shiftPressed && event->button() == Qt::RightButton) {
            startNodeIndex = -1;
            QPointF mousePos = graphicsView->mapToScene(event->pos());
            int nodeIndex;
            if (isMouseOverNode(mousePos, nodeIndex)) {
                rightClickedNodeIndex = nodeIndex;
                contextMenuOpened = true;
                contextMenu->exec(QCursor::pos());
            }
        } else if (!contextMenuOpened && event->button() == Qt::LeftButton) {
            startPos = graphicsView->mapToScene(event->pos());
            int nodeIndex;
            if (isMouseOverNode(startPos, nodeIndex))
                startNodeIndex = nodeIndex;
        }
#ifdef DEBUG
        else if (!contextMenuOpened && !shiftPressed && event->button() == Qt::RightButton) {
            QPointF scenePos = graphicsView->mapToScene(event->pos());
            int nodeIndex;
            if (isMouseOverNode(scenePos, nodeIndex))
                return;
            if (scenePos.x() < 0 || scenePos.y() < 0) {
                qDebug() << "Invalid scene coordinates, skipping node creation.";
                return;
            }
            Node *newNode = new Node(scenePos.x(), scenePos.y(), "New Node", font(), NodeShapes::Square);
            newNode->setModified(true);
            graphicsScene->addItem(newNode);
            nodes.append(newNode);
        }
#endif
        if (contextMenuOpened)
            contextMenuOpened = false;
    }
}

void MainWindow::removeConnections() {
    if (rightClickedNodeIndex != -1 && IS_VALID_INDEX(rightClickedNodeIndex, nodes)) {
        Node *nodeToRemove = nodes[rightClickedNodeIndex];
        QVector<int> indicesToRemove, connectedNodes;
        REMOVE_NODES_CONNECTIONS(rightClickedNodeIndex, connections, indicesToRemove, connectedNodes);
        for (int i = indicesToRemove.size() - 1; i >= 0; --i) {
            connections.removeAt(indicesToRemove[i]);
        }
        for (int i : connectedNodes) {
            if (IS_VALID_INDEX(i, nodes)) {
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
void MainWindow::showDialog(const QString &labelText, QWidget *inputWidget, std::function<void()> onAccept) {
    QDialog dialog(this);
    dialog.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    dialog.setMinimumSize(200, 120);
    dialog.setMaximumSize(600, 120);
    QVBoxLayout layout(&dialog);
    QLabel label(labelText);
    QPushButton okButton(tr("OK"));
    QPushButton cancelButton(tr("Cancel"));
    addWidgets(layout, &label, inputWidget, &okButton, &cancelButton);
    connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(&cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted)
        onAccept();
}

void MainWindow::renameSelectedNode() {
    if (rightClickedNodeIndex != -1 && rightClickedNodeIndex >= 0 && rightClickedNodeIndex < nodes.size()) {
        Node *node = nodes[rightClickedNodeIndex];
        QLineEdit *lineEdit = new QLineEdit(node->getName());
        auto onAccept = [this, node, lineEdit]() {
            QString newName = lineEdit->text();
            if (newName.length() > 100) {
                QMessageBox::warning(this, tr("Invalid Name"), tr("The name cannot exceed 100 characters."));
                return;
            }
            if (!newName.isEmpty()) {
                node->setName(newName);
                node->setModified(true); // Mark the node as modified
                updateDisplay();         // Update the display if necessary
            }
        };
        showDialog(tr("New name:"), lineEdit, onAccept);
    } else {
        qDebug() << "Invalid node index in renameSelectedNode.";
    }
    simulateKeyPress(Qt::Key_Shift);
    simulateKeyRelease(Qt::Key_Shift);
}

void MainWindow::changeNodeShape() {
    if (rightClickedNodeIndex != -1 && rightClickedNodeIndex < nodes.size()) {
        Node *node = nodes[rightClickedNodeIndex];
        QComboBox *comboBox = new QComboBox();
        comboBox->addItems({"Circle", "Square", "Triangle"});
        auto onAccept = [this, node, comboBox]() {
            QString selectedShape = comboBox->currentText();
            if (!selectedShape.isEmpty()) {
                NodeShapes newShape = stringToShape(selectedShape);
                node->setShape(newShape);
                node->setModified(true);
                updateDisplay();
            }
        };
        showDialog(tr("Shape:"), comboBox, onAccept);
    }
    simulateKeyPress(Qt::Key_Shift);
    simulateKeyRelease(Qt::Key_Shift);
}
void MainWindow::changeNodeColor() {
    if (rightClickedNodeIndex >= 0 && rightClickedNodeIndex < nodes.size()) {
        Node *node = nodes[rightClickedNodeIndex];
        QColorDialog colorDialog(node->getColor(), this);
        colorDialog.setOption(QColorDialog::DontUseNativeDialog);
        colorDialog.setWindowTitle(tr("Select Color"));
        colorDialog.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        if (colorDialog.exec() == QColorDialog::Accepted) {
            QColor selectedColor = colorDialog.currentColor();
            if (selectedColor.isValid()) {
                node->setColor(selectedColor);
                node->setModified(true);
                updateDisplay();
            }
        }
    }
    simulateKeyPress(Qt::Key_Shift);
    simulateKeyRelease(Qt::Key_Shift);
}
void MainWindow::associateStarToNode() {
    if (rightClickedNodeIndex != -1 && rightClickedNodeIndex < nodes.size()) {
        Node *node = nodes[rightClickedNodeIndex];
        QComboBox *courseComboBox = new QComboBox(this);
        for (const QString &courseName : courseNames) {
            if (!associatedCourses.contains(courseName))
                courseComboBox->addItem(courseName);
        }
        auto onAccept = [this, node, courseComboBox] {
            QString selectedCourse = courseComboBox->currentText();
            QString oldCourse = node->getAssociatedCourse();
            if (!oldCourse.isEmpty() && associatedCourses.contains(oldCourse))
                associatedCourses.removeAll(oldCourse);
            node->setStarAssociated(true);
            node->setModified(true);
            node->setAssociatedCourse(selectedCourse);
            if (!associatedCourses.contains(selectedCourse))
                associatedCourses.append(selectedCourse);
            updateDisplay();
            delete courseComboBox;
        };
        showDialog(tr("Select Course to Associate"), courseComboBox, onAccept);
    } else {
        qDebug() << "Invalid node index in associateStarToNode.";
    }
    simulateKeyPress(Qt::Key_Shift);
    simulateKeyRelease(Qt::Key_Shift);
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
Node *MainWindow::findAssociatedNode() {
    REPA(Node, nodes, isStarAssociated());
    return nullptr;
}
void MainWindow::toggleStarDisplay() {
    showStarDisplay = !showStarDisplay;
    QTabWidget *tabWidget = findChild<QTabWidget *>("tabWidget");
    if (showStarDisplay) {
        textUpdate();
        HIDE_WIDGETS(settingsButton, switchViewButton, tabWidget, graphicsView, saveButton);
        SHOW_WIDGETS(emulatorText, b3313Text);
        REPA(Node, nodes, hide());
        HIDE_WIDGETS(emulatorText, b3313Text); // Hide again for some condition
    } else {
        HIDE_WIDGETS(settingsButton, tabWidget);
        REMOVE_ALL_TABS(tabWidget);
        SHOW_WIDGETS(graphicsView, saveButton, switchViewButton, settingsButton);
        HIDE_WIDGETS(emulatorText, b3313Text);
        REPA(Node, nodes, show());
        updateMinimap();
        syncMinimapView();
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
    setWindowResizable(settingsWindow->isResizable());
    if (force_toggle_star_display) {
        toggleStarDisplay();
        force_toggle_star_display = false;
    }
    REMOVE_ITEMS_OF_TYPE(graphicsScene, QGraphicsLineItem);
    REMOVE_ITEMS_OF_TYPE(graphicsScene, QGraphicsPolygonItem);
    REMOVE_ITEMS_OF_TYPE(graphicsScene, QGraphicsPixmapItem);
    if (showStarDisplay) {
        textUpdate();
        jsonLoaderThread->loadJson(GLOBAL_STAR_DISPLAY_JSON_STR);
        updateStarDisplay();
        SHOW_WIDGETS(switchViewButton);
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
                QPointF startEdgePoint = getNodeEdgePoint(startNode, endNode->pos());
                QPointF endEdgePoint = getNodeEdgePoint(endNode, startNode->pos());
                QGraphicsLineItem *lineItem = new QGraphicsLineItem(startEdgePoint.x(), startEdgePoint.y(), endEdgePoint.x(), endEdgePoint.y());
                lineItem->setPen(QPen(Qt::black));
                graphicsScene->addItem(lineItem);
                QLineF line(startEdgePoint, endEdgePoint);
                double angle = std::atan2(-line.dy(), line.dx());
                QPointF arrowP1 = endEdgePoint - QPointF(std::sin(angle + M_PI / 3) * 10, std::cos(angle + M_PI / 3) * 10);
                QPointF arrowP2 = endEdgePoint - QPointF(std::sin(angle + M_PI - M_PI / 3) * 10, std::cos(angle + M_PI - M_PI / 3) * 10);
                QPolygonF arrowHead;
                arrowHead << endEdgePoint << arrowP1 << arrowP2;
                QGraphicsPolygonItem *arrowItem = new QGraphicsPolygonItem(arrowHead);
                arrowItem->setBrush(Qt::black);
                graphicsScene->addItem(arrowItem);
            } else {
                qWarning() << "Connection has invalid node index:" << conn;
            }
        }
        for (Node *node : nodes) {
            if (node->isStarAssociated()) {
                QGraphicsPixmapItem *starIcon = new QGraphicsPixmapItem(QPixmap("resources/textures/associated_to_node.png"));
                starIcon->setPos(node->pos() + QPointF(node->boundingRect().width() / 2, -node->boundingRect().height() / 2));
                starIcon->setScale(0.15);
                graphicsScene->addItem(starIcon);
            }
        }
        updateMinimap();
        isModified();
    }
    syncMinimapView();
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
void MainWindow::generateTabContent(const QString &tabName, const QPixmap &pixmap, QWidget *contentWidget, QVBoxLayout *contentLayout) {
    while (QLayoutItem *item = contentLayout->takeAt(0)) {
        if (item->widget())
            delete item->widget();
        delete item;
    }
    auto tabLabel = std::make_unique<QLabel>(tabName, contentWidget);
    tabLabel->setFont(this->font());
    tabLabel->setStyleSheet("color: black;");
    contentLayout->addWidget(tabLabel.release());
    auto pixmapLabel = std::make_unique<QLabel>(contentWidget);
    pixmapLabel->setPixmap(pixmap);
    contentLayout->addWidget(pixmapLabel.release());
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
}

void MainWindow::initializeStarDisplay(const QJsonObject &jsonData) {
    star_display_json_data = jsonData;
    associatedCourseNames.clear();
    for (Node *node : nodes) {
        if (node->isStarAssociated()) {
            QString associatedCourse = node->getAssociatedCourse();
            if (!associatedCourse.isEmpty())
                associatedCourseNames.append(associatedCourse);
        }
    }

    std::string saveLocation = GetParallelLauncherSaveLocation();
    QJsonObject format = jsonData["format"].toObject();
    star_diplay_params.saveFormat = parseSaveFormat(format["save_type"].toString().toStdString());
    star_diplay_params.slotsStart = format["slots_start"].toInt();
    star_diplay_params.slotSize = format["slot_size"].toInt();
    star_diplay_params.activeBit = format["active_bit"].toInt();
    star_diplay_params.numSlots = format["num_slots"].toInt();
    star_diplay_params.checksumOffset = format["checksum_offset"].toInt();

    saveData = ReadSrmFile(saveLocation, star_diplay_params);
    if (saveData.empty() || star_diplay_params.numSlots <= 0) {
        qWarning() << "Save data is empty or numSlots is not valid.";
        return;
    }
    tabNames.clear();
    for (int i = 0; i < static_cast<int>(star_diplay_params.numSlots); ++i) {
        tabNames.append("Mario " + QString::number(i));
    }
}

void MainWindow::updateStarDisplay() {
    if (!isRomHackLoaded(global_detected_emulator)) {
        HIDE_WIDGETS(tabWidget);
        SHOW_WIDGETS(emulatorText, b3313Text, switchViewButton, settingsButton);
        return;
    }
    SHOW_WIDGETS(tabWidget, switchViewButton, settingsButton);
    HIDE_WIDGETS(emulatorText, b3313Text);
    if (tabNames.isEmpty() || starCollectedTexture.isNull() || starMissingTexture.isNull())
        return;
    int associatedCourseYPosition = 0;
    for (int i = 0; i < static_cast<int>(star_diplay_params.numSlots); ++i) {
        QString tabName = tabNames[i];
        QWidget *existingTab = nullptr;
        for (int j = 0; j < tabWidget->count(); ++j) {
            if (tabWidget->tabText(j) == tabName) {
                existingTab = tabWidget->widget(j);
                break;
            }
        }
        QWidget *contentWidget;
        QVBoxLayout *contentLayout;
        bool isNewTab = !existingTab;
        if (isNewTab) {
            auto tabContent = std::make_unique<QWidget>();
            auto layout = std::make_unique<QVBoxLayout>(tabContent.get());
            contentWidget = new QWidget();
            contentLayout = new QVBoxLayout(contentWidget);
            contentWidget->setLayout(contentLayout);
            scrollArea = new QScrollArea(tabWidget);
            scrollArea->setWidgetResizable(true);
            scrollArea->setWidget(contentWidget);
            tabWidget->addTab(scrollArea, tabName);
        } else {
            scrollArea = qobject_cast<QScrollArea *>(existingTab);
            contentWidget = scrollArea->widget();
            contentLayout = qobject_cast<QVBoxLayout *>(contentWidget->layout());
        }
        float collectedHeight = static_cast<float>(starCollectedTexture.height());
        float missingHeight = static_cast<float>(starMissingTexture.height());
        float starTextureHeight = std::max(collectedHeight, missingHeight);
        int yOffset = 0, reservedHeight = 50;
        QMap<QString, QMap<QString, QVector<StarData>>> groupCourseMap = JsonLoading::readStarDisplayJsonData(star_display_json_data, saveData, star_diplay_params, i);
        for (auto groupIt = groupCourseMap.cbegin(); groupIt != groupCourseMap.cend(); ++groupIt) {
            const QMap<QString, QVector<StarData>> &courseStarsMap = groupIt.value();
            for (auto courseIt = courseStarsMap.cbegin(); courseIt != courseStarsMap.cend(); ++courseIt) {
                if (jump_to_star_display_associated_line && courseIt.key() == jump_to_which_line)
                    associatedCourseYPosition = yOffset;
                yOffset += std::max(static_cast<int>(starTextureHeight), 30);
            }
            yOffset += 10;
        }

        int totalHeight = yOffset + reservedHeight;
        QPixmap pixmap(graphicsView->width(), totalHeight);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        yOffset = 0;
        int rectLeft = 50, rectTop = 50;
        QImage logoTexture = ImageCache::getImage("resources/textures/associated_to_node.png");
        QImage scaledLogoTexture = logoTexture.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        float starSpacing = 64.0f;
        float logoHeight = static_cast<float>(scaledLogoTexture.height());
        float logoWidth = static_cast<float>(scaledLogoTexture.width());
        float maxLineHeight = 30.0f;
        float scrollOffsetY = scrollArea->widget()->pos().y();
        for (auto groupIt = groupCourseMap.cbegin(); groupIt != groupCourseMap.cend(); ++groupIt) {
            const QString &groupName = groupIt.key();
            const QMap<QString, QVector<StarData>> &courseStarsMap = groupIt.value();
            QFont groupFont = painter.font();
            groupFont.setBold(true);
            painter.setFont(groupFont);
            QRectF groupTextRect(rectLeft + 10, rectTop + yOffset - scrollOffsetY, 600, 30);
            painter.setPen(Qt::blue);
            painter.drawText(groupTextRect, groupName);
            yOffset += std::max(static_cast<int>(starTextureHeight), 30);
            for (auto it = courseStarsMap.cbegin(); it != courseStarsMap.cend(); ++it) {
                const QString &courseName = it.key();
                const QVector<StarData> &stars = it.value();
                QRectF logoRect(rectLeft + 10, rectTop + yOffset - scrollOffsetY, logoWidth, logoHeight);
                if (associatedCourseNames.contains(courseName)) {
                    painter.drawImage(logoRect, scaledLogoTexture);
                    logoRects[courseName] = logoRect;
                }
                QRectF courseTextRect(logoRect.right() + 10, rectTop + yOffset - scrollOffsetY, 600, 30);
                painter.setFont(painter.font());
                painter.setPen(Qt::white);
                painter.drawText(courseTextRect, courseName);
                float currentX = courseTextRect.right() + 10;
                float lineHeight = std::max(starTextureHeight, maxLineHeight);
                float maxRightX = currentX;
                for (const auto &star : stars) {
                    for (int i = 0; i < star.numStars; ++i) {
                        QRectF starRect(
                            currentX + i * starSpacing,
                            courseTextRect.top() + (lineHeight - starTextureHeight) / 2,
                            starCollectedTexture.width(),
                            starCollectedTexture.height());
                        const QImage &starTexture = star.collected ? starCollectedTexture : starMissingTexture;
                        painter.drawImage(starRect, starTexture);
                        if (starRect.right() > maxRightX)
                            maxRightX = starRect.right();
                    }
                    currentX += starSpacing * star.numStars;
                }
                QRectF boundingRect(
                    rectLeft, rectTop + yOffset - scrollOffsetY,
                    maxRightX - rectLeft + 10,
                    lineHeight);
                courseNameRects[courseName] = boundingRect;
                painter.setPen(QPen(Qt::black, 2));
                painter.drawRect(boundingRect);
                yOffset += lineHeight;
            }
            yOffset += 10;
        }
        painter.end();

        if (isNewTab)
            generateTabContent(tabName, pixmap, contentWidget, contentLayout);
        else
            contentWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

        contentWidget->setMinimumHeight(totalHeight);

        if (jump_to_star_display_associated_line && associatedCourseYPosition != -1) {
            scrollArea->verticalScrollBar()->setValue(associatedCourseYPosition);
            jump_to_star_display_associated_line = false;
        }
    }
}
QStringList MainWindow::getCourseNamesFromSlot0(const QJsonObject &jsonData) {
    QStringList courseNames;
    if (!jsonData.contains("groups") || !jsonData["groups"].isArray()) {
        qWarning() << "jsonData does not contain valid 'groups' array.";
        return courseNames;
    }
    QJsonArray groupsArray = jsonData["groups"].toArray();
    for (const auto &groupValue : groupsArray) {
        QJsonObject group = groupValue.toObject();
        if (!group.contains("courses") || !group["courses"].isArray())
            continue;
        QJsonArray coursesArray = group["courses"].toArray();
        for (const auto &courseValue : coursesArray) {
            QJsonObject course = courseValue.toObject();
            if (course.contains("name")) {
                QString courseName = course["name"].toString();
                courseNames.append(courseName);
            }
        }
    }
    return courseNames;
}
#include "MainWindow.moc"