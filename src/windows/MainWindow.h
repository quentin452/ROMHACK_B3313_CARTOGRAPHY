#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H
#include <windows.h>

#include <psapi.h>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <nlohmann/json.hpp>
#include <romhack_b3313_cartography/utils/defines.hpp>


#include <romhack_b3313_cartography/utils/rom_utils.h>

#include <romhack_b3313_cartography/uis/Textures.h>

#include "../uis/TabManager.h"
#include <romhack_b3313_cartography/uis/Node.h>
#include <romhack_b3313_cartography/uis/StarDisplay.h>

#include <QComboBox>
#include <QDebug>
#include <QFile>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPair>
#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QVector>

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow();

  protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void setNodesMovable(bool movable);

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private slots:
    void removeConnections();
    void saveNodes();

    void toggleStarDisplay();

  private:
    void closeEvent(QCloseEvent *event) override;

    bool isModified() const;
    void onTimerUpdate();
    void loadJsonData(const QString &filename);
    void parseJsonData(const QJsonObject &jsonData);
    void updateDisplay(const QJsonObject &jsonData);

    void displayStars(const QJsonObject &jsonData);

    bool isMouseOverNode(const QPointF &mousePos, int &nodeIndex);
    QVector<Node *> loadNodes(const QString &filename, QFont &font);
    QGraphicsView *graphicsView;
    QGraphicsScene *graphicsScene;
    QVector<Node *> nodes;
    QVector<QPair<int, int>> connections;
    QPointF startPos;
    bool dragging = false;
    int startNodeIndex = -1;
    TabManager *tabManager;
    QStringList tabNames;
    QJsonObject lastJsonData;
    bool showStarDisplay = false; // Contrôle pour afficher l'affichage des étoiles
    StarDisplay starDisplay;      // Instance de la classe StarDisplay
    QList<Node *> mind_map_nodes;
    QGraphicsTextItem *emulatorText = nullptr;
    QGraphicsTextItem *b3313Text = nullptr;
    QComboBox *dropdownMenu = nullptr;
    QPushButton *saveButton = nullptr;
    QPushButton *switchViewButton = nullptr;
    QTimer *updateTimer = nullptr;
    Node *startArrowNode = nullptr; // Node where the arrow starts
    bool shiftPressed = false;      // Indicates if Shift key is pressed
    QGraphicsLineItem *currentArrow = nullptr;
    QMenu *contextMenu;
    int rightClickedNodeIndex = -1;
    const int WIDTH = 1280;
    const int HEIGHT = 720;
    std::wstring global_detected_emulator;
};
#endif // MAIN_WINDOW_H