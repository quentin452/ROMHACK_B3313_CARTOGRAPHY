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
bool MainWindow::shiftPressed = false, MainWindow::showStarDisplay = false, MainWindow::force_toggle_star_display = false, MainWindow::jump_to_star_display_associated_line = false;
int MainWindow::startNodeIndex = -1;
QVector<QPair<int, int>> MainWindow::connections;
QGraphicsScene *MainWindow::graphicsScene = nullptr;
QJsonObject MainWindow::lastJsonData;
QStringList MainWindow::courseNames, MainWindow::associatedCourses;
QString MainWindow::jump_to_which_line;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      settingsWindow(nullptr) {
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
    ADD_ACTION(contextMenu, associateStarAction, associateStarToNode)
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
    QJsonObject jsonData = JsonLoading::loadJsonData2("resources/stars_layout/b3313-V1.0.2/star_display_layout.json");
    courseNames = getCourseNamesFromSlot0(jsonData);
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
    if (showStarDisplay) {
        QPointF pos = event->pos();
        QString courseClicked;

        for (auto it = courseNameRects.cbegin(); it != courseNameRects.cend(); ++it) {
            if (it.value().contains(pos)) {
                courseClicked = it.key();
                break;
            }
        }

        if (courseClicked.isEmpty()) {
            for (auto it = logoRects.cbegin(); it != logoRects.cend(); ++it) {
                if (it.value().contains(pos)) {
                    courseClicked = it.key();
                    break;
                }
            }
        }

        if (!courseClicked.isEmpty()) {
            for (Node *node : MainWindow::nodes) {
                if (node->getAssociatedCourse() == courseClicked) {
                    toggleStarDisplay();
                    graphicsView->centerOn(node->pos());
                    break;
                }
            }
        }
    } else {
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
        QComboBox *courseComboBox = new QComboBox(this); // Utilisation de QComboBox directement
        for (const QString &courseName : courseNames) {
            if (!associatedCourses.contains(courseName)) {
                courseComboBox->addItem(courseName);
            }
        }

        auto onAccept = [this, node, courseComboBox] {
            QString selectedCourse = courseComboBox->currentText();
            QString oldCourse = node->getAssociatedCourse();
            if (!oldCourse.isEmpty() && associatedCourses.contains(oldCourse)) {
                associatedCourses.removeAll(oldCourse);
            }
            node->setStarAssociated(true);
            node->setModified(true);
            node->setAssociatedCourse(selectedCourse);
            if (!associatedCourses.contains(selectedCourse)) {
                associatedCourses.append(selectedCourse);
            }
            updateDisplay();
            delete courseComboBox; // Supprimer le combo box
        };

        // Passer le QComboBox et la fonction lambda à showDialog
        showDialog(tr("Select Course to Associate"), courseComboBox, onAccept);
    } else {
        qDebug() << "Invalid node index in associateStarToNode.";
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
Node *MainWindow::findAssociatedNode() {
    for (Node *node : nodes) {
        if (node->isStarAssociated()) {
            return node;
        }
    }
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
        Node *associatedNode = findAssociatedNode();
        if (associatedNode) {
            graphicsView->centerOn(associatedNode->pos());
        }
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
    if (force_toggle_star_display) {
        toggleStarDisplay();
        // if (!showStarDisplay)
        //      graphicsView->centerOn(node->pos());
        force_toggle_star_display = false;
    }
    if (showStarDisplay) {
        textUpdate();
        QJsonObject jsonData = JsonLoading::loadJsonData2("resources/stars_layout/b3313-V1.0.2/star_display_layout.json"); // NEED OPTIMIZATIONS
        displayStars(jsonData);
        switchViewButton->show();
    } else {
        REMOVE_ITEMS_OF_TYPE(graphicsScene, QGraphicsLineItem);
        REMOVE_ITEMS_OF_TYPE(graphicsScene, QGraphicsPolygonItem);
        REMOVE_ITEMS_OF_TYPE(graphicsScene, QGraphicsPixmapItem);
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
                graphicsScene->addItem(starIcon);
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
void MainWindow::drawCourseStars(QPainter &painter,
                                 const QMap<QString, QMap<QString, QVector<StarData>>> &groupCourseMap,
                                 float startX, float starTextureHeight,
                                 float rectLeft, float rectTop,
                                 int &yOffset, int reservedHeight,
                                 const QImage &starCollectedTexture,
                                 const QImage &starMissingTexture,
                                 const QStringList &associatedCourseNames,
                                 const QImage &logoTexture,
                                 QMap<QString, QRectF> &courseNameRects,
                                 QMap<QString, QRectF> &logoRects) {

    float starSpacing = 64.0f;
    float logoHeight = static_cast<float>(logoTexture.height());
    float logoWidth = static_cast<float>(logoTexture.width());

    for (auto groupIt = groupCourseMap.cbegin(); groupIt != groupCourseMap.cend(); ++groupIt) {
        const QString &groupName = groupIt.key();
        const QMap<QString, QVector<StarData>> &courseStarsMap = groupIt.value();

        QFont groupFont = painter.font();
        groupFont.setBold(true);
        painter.setFont(groupFont);
        QRectF groupTextRect(rectLeft + 10, rectTop + yOffset, 600, 30);
        painter.setPen(Qt::blue);
        painter.drawText(groupTextRect, groupName);

        yOffset += std::max(static_cast<int>(starTextureHeight), 30);

        for (auto it = courseStarsMap.cbegin(); it != courseStarsMap.cend(); ++it) {
            const QString &courseName = it.key();
            const QVector<StarData> &stars = it.value();

            QFont font = painter.font();
            QRectF courseTextRect(rectLeft + 10, rectTop + yOffset, 600, 30);
            painter.setFont(font);
            painter.setPen(Qt::white);
            painter.drawText(courseTextRect, courseName);
            courseNameRects[courseName] = courseTextRect;

            if (associatedCourseNames.contains(courseName)) {
                QRectF logoRect(courseTextRect.right() + 10, courseTextRect.top(), logoWidth, logoHeight);
                painter.drawImage(logoRect, logoTexture);
                logoRects[courseName] = logoRect;
            }

            float currentX = courseTextRect.right() + 10;
            for (const auto &star : stars) {
                for (int i = 0; i < star.numStars; ++i) {
                    QRectF starRect(
                        currentX + i * starSpacing,
                        courseTextRect.top() + (30 - starTextureHeight) / 2,
                        starCollectedTexture.width(),
                        starCollectedTexture.height());
                    const QImage &starTexture = star.collected ? starCollectedTexture : starMissingTexture;
                    painter.drawImage(starRect, starTexture);
                }
                currentX += starSpacing * star.numStars;
            }

            yOffset += std::max(static_cast<int>(starTextureHeight), 30);
        }
        yOffset += 10;
    }
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

// Fonction principale pour afficher les étoiles
void MainWindow::displayStars(const QJsonObject &jsonData) {
    if (!isRomHackLoaded(global_detected_emulator)) {
        tabWidget->hide();
        emulatorText->show();
        b3313Text->show();
        switchViewButton->show();
        settingsButton->show();
        return;
    }
    QStringList associatedCourseNames;
    for (Node *node : nodes) {
        if (node->isStarAssociated()) {
            QString associatedCourse = node->getAssociatedCourse();
            if (!associatedCourse.isEmpty()) {
                associatedCourseNames.append(associatedCourse);
            }
        }
    }
    tabWidget->show();
    emulatorText->hide();
    b3313Text->hide();
    switchViewButton->show();
    settingsButton->show();
    std::string saveLocation = GetParallelLauncherSaveLocation();
    QJsonObject format = jsonData["format"].toObject();
    SaveParams params;
    params.saveFormat = parseSaveFormat(format["save_type"].toString().toStdString());
    params.slotsStart = format["slots_start"].toInt();
    params.slotSize = format["slot_size"].toInt();
    params.activeBit = format["active_bit"].toInt();
    params.numSlots = format["num_slots"].toInt();
    params.checksumOffset = format["checksum_offset"].toInt();
    auto saveData = ReadSrmFile(saveLocation, params);
    if (saveData.empty() || params.numSlots <= 0)
        return;
    tabNames.clear();
    for (int i = 0; i < static_cast<int>(params.numSlots); ++i) {
        tabNames.append("Mario " + QString::number(i));
    }
    QImage starCollectedTexture("resources/textures/star-collected.png");
    QImage starMissingTexture("resources/textures/star-missing.png");
    QImage logoTexture("resources/textures/associated_to_node.png");
    if (starCollectedTexture.isNull() || starMissingTexture.isNull()) {
        qWarning() << "One or both star textures failed to load.";
        return;
    }
    float collectedHeight = static_cast<float>(starCollectedTexture.height());
    float missingHeight = static_cast<float>(starMissingTexture.height());
    float starTextureHeight = std::max(collectedHeight, missingHeight);

    int associatedCourseYPosition = -1; // Track the Y position of the associated course

    for (int i = 0; i < static_cast<int>(params.numSlots); ++i) {
        QString tabName = tabNames[i];
        QWidget *existingTab = nullptr;
        for (int j = 0; j < tabWidget->count(); ++j) {
            if (tabWidget->tabText(j) == tabName) {
                existingTab = tabWidget->widget(j);
                break;
            }
        }
        QScrollArea *scrollArea;
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
        int yOffset = 0;
        int reservedHeight = 50;
        QMap<QString, QMap<QString, QVector<StarData>>> groupCourseMap = JsonLoading::readStarDisplayJsonData(jsonData, saveData, params, i);
        for (auto groupIt = groupCourseMap.cbegin(); groupIt != groupCourseMap.cend(); ++groupIt) {
            const QMap<QString, QVector<StarData>> &courseStarsMap = groupIt.value();
            for (auto courseIt = courseStarsMap.cbegin(); courseIt != courseStarsMap.cend(); ++courseIt) {
                if (jump_to_star_display_associated_line && courseIt.key() == jump_to_which_line) {
                    associatedCourseYPosition = yOffset; // Set position of the associated course
                }
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
        drawCourseStars(painter, groupCourseMap, 50 + 10, starTextureHeight, 50, 50, yOffset, reservedHeight, starCollectedTexture, starMissingTexture, associatedCourseNames, logoTexture, courseNameRects, logoRects);
        painter.end();
        if (isNewTab)
            generateTabContent(tabName, pixmap, contentWidget, contentLayout);
        else
            contentWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        contentWidget->setMinimumHeight(totalHeight);

        // Scroll to the position of the associated course
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
