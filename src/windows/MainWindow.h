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

#include "../utils/JsonLoading.h"
#include "SettingsWindow.h"
class SettingsWindow; // Forward declaration
class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = nullptr);
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
    static QJsonObject lastJsonData;

    static bool shiftPressed;
    static int startNodeIndex;
    static QVector<Node *> nodes;
    static QVector<QPair<int, int>> connections;
    static QGraphicsScene *graphicsScene;
    // QLabel *fpsLabel;

  protected:
    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void setNodesMovable(bool movable);

    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private slots:
    void removeConnections();
    void saveNodes();
    void toggleStarDisplay();
    void openSettingsWindow();

  private:
    bool contextMenuOpened = false;
    void closeEvent(QCloseEvent *event) override;
    void textUpdate();
    bool isModified() const;
    void onTimerUpdate();
    void updateDisplay();
    void renameSelectedNode();
    SettingsWindow *settingsWindow = nullptr;
    QPointF startPos;
    bool showStarDisplay = false; // Contrôle pour afficher l'affichage des étoiles
    StarDisplay starDisplay;      // Instance de la classe StarDisplay
    QList<Node *> mind_map_nodes;

    QPushButton *saveButton = nullptr;
    QTimer *updateTimer = nullptr, *refresh_rate_timer = nullptr;
    Node *startArrowNode = nullptr; // Node where the arrow starts
    QGraphicsLineItem *currentArrow = nullptr;
    QMenu *contextMenu = nullptr;
    const int WIDTH = 1280, HEIGHT = 720;
    std::unique_ptr<MainWindowUpdateThread> thread;
    QWidget *star_display_centralWidget = nullptr;
    QStackedWidget *stackedWidget = nullptr;
    QWidget *centralWidgetZ = nullptr;
    QScrollArea *scrollArea_star_display = nullptr;
    int stardisplayscrollPosition = 0, rightClickedNodeIndex = -1;
    QPushButton *settingsButton = nullptr;
#ifdef DEBUG
    QString b33_13_mind_map_str = "b3313-v1.0.2-Mind_map.json";
#else
    QString b33_13_mind_map_str = "stars_layout/b3313-V1.0.2/b3313-v1.0.2-Mind_map.json";
#endif
};

#endif // MAIN_WINDOW_H