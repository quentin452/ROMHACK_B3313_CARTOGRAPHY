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

#include <romhack_b3313_cartography/uis/Node.h>
#include <romhack_b3313_cartography/uis/StarDisplay.h>

#include "../uis/MouseFixGraphicScene.h"
#include "MainWindowUpdateThread.hpp"
#include <memory>
#include <romhack_b3313_cartography/utils/qt_includes.hpp>

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow();
    ~MainWindow();

    static bool isMouseOverNode(const QPointF &mousePos, int &nodeIndex);

    static std::wstring global_detected_emulator;
    static QLabel *emulatorText, *b3313Text;
    static QStringList tabNames;
    static QGraphicsView *graphicsView;
    static QTabWidget *tabWidget;
    static QFont qfont;
    static QVBoxLayout *star_display_mainLayout;
    static QPushButton *switchViewButton;

    static bool shiftPressed;
    static int startNodeIndex;
    static QVector<Node *> nodes;
    static QVector<QPair<int, int>> connections;
    static QGraphicsScene *graphicsScene;

  protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void setNodesMovable(bool movable);

    void mousePressEvent(QMouseEvent *event) override;

  private slots:
    void removeConnections();
    void saveNodes();

    void toggleStarDisplay();

  private:
    bool contextMenuOpened = false;
    void closeEvent(QCloseEvent *event) override;
    void textUpdate();
    bool isModified() const;
    void onTimerUpdate();
    void updateScrollbarpos();
    void loadJsonData(const QString &filename);
    QJsonObject loadJsonData2(const QString &filePath);
    void parseJsonData(const QJsonObject &jsonObj);
    void updateDisplay();
    void printWidgetOrder();

    QPointF startPos;
    QJsonObject lastJsonData;
    bool showStarDisplay = false; // Contrôle pour afficher l'affichage des étoiles
    StarDisplay starDisplay;      // Instance de la classe StarDisplay
    QList<Node *> mind_map_nodes;

    QPushButton *saveButton = nullptr;
    QTimer *updateTimer = nullptr;
    Node *startArrowNode = nullptr; // Node where the arrow starts
    QGraphicsLineItem *currentArrow = nullptr;
    QMenu *contextMenu = nullptr;
    int rightClickedNodeIndex = -1;
    const int WIDTH = 1280;
    const int HEIGHT = 720;
    std::unique_ptr<MainWindowUpdateThread> thread;
    QWidget *star_display_centralWidget = nullptr;
    QStackedWidget *stackedWidget = nullptr;
    QWidget *centralWidgetZ = nullptr;
    QScrollArea *scrollArea_star_display = nullptr;
    int stardisplayscrollPosition = 0;
};
#endif // MAIN_WINDOW_H