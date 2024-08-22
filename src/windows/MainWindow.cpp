#include "MainWindow.h"
std::basic_string<wchar_t> MainWindow::global_detected_emulator;
QLabel *MainWindow::emulatorText, *MainWindow::b3313Text = nullptr;
QStringList MainWindow::tabNames;
QGraphicsView *MainWindow::graphicsView = nullptr;
QTabWidget *MainWindow::tabWidget = nullptr;
QFont MainWindow::qfont;
QVBoxLayout *MainWindow::star_display_mainLayout = nullptr;
QPushButton *MainWindow::switchViewButton = nullptr, *MainWindow::settingsButton = nullptr;

QVector<Node *> MainWindow::nodes;
bool MainWindow::shiftPressed = false;
int MainWindow::startNodeIndex = -1;
QVector<QPair<int, int>> MainWindow::connections;
QGraphicsScene *MainWindow::graphicsScene = nullptr;
QJsonObject MainWindow::lastJsonData;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), settingsWindow(nullptr) {
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
    saveButton = new QPushButton("Save", this);
    switchViewButton = new QPushButton("Switch View", this);
    addWidgets(*layout, emulatorText, b3313Text, graphicsView, saveButton, switchViewButton);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveNodes);
    connect(switchViewButton, &QPushButton::clicked, this, &MainWindow::toggleStarDisplay);
    contextMenu = new QMenu(this);
    ADD_ACTION(contextMenu, renameNodeAction, renameSelectedNode)
    ADD_ACTION(contextMenu, removeConnectionsAction, removeConnections)
    ADD_ACTION(contextMenu, changeShapeAction, changeNodeShape)
    ADD_ACTION(contextMenu, changeColorAction, changeNodeColor)
    HIDE_WIDGETS(emulatorText, b3313Text);
    thread = std::make_unique<MainWindowUpdateThread>(this);
    connect(thread.get(), &MainWindowUpdateThread::updateNeeded, this, &MainWindow::onTimerUpdate);
    thread->start();
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
    tabWidget->hide();
    qfont = this->font();
    settingsWindow = new SettingsWindow(this);
    setWindowResizable(settingsWindow->isResizable());
    settingsButton = new QPushButton("Settings", this);
    star_display_mainLayout->addWidget(settingsButton);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::openSettingsWindow);
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
    if (thread) {
        thread->stop();
        thread->wait();
    }
    delete settingsWindow;
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
    if (event->key() == Qt::Key_S && event->modifiers() & Qt::ControlModifier)
        saveNodes();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        shiftPressed = false;
        REPA(Node, nodes, setMovable(true))
        startNodeIndex = -1;
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
    QPointF mousePosScene = graphicsView->mapToScene(event->pos());
    qreal margin = 333.0;

    QRectF sceneRect = graphicsScene->sceneRect().adjusted(margin, margin, -margin, -margin);
    if (!sceneRect.contains(mousePosScene)) {
        return;
    }
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
        if (isMouseOverNode(scenePos, nodeIndex)) {
            return;
        }

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
    if (showStarDisplay) {
        textUpdate();
        QJsonObject jsonData = JsonLoading::loadJsonData2("resources/stars_layout/b3313-V1.0.2/star_display_layout.json"); // NEED OPTIMIZATIONS
        starDisplay.displayStars(jsonData);
        switchViewButton->show();
    } else {
        REMOVE_ITEMS_OF_TYPE(graphicsScene, QGraphicsLineItem);
        REMOVE_ITEMS_OF_TYPE(graphicsScene, QGraphicsPolygonItem);

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
