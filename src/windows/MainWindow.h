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
#include <romhack_b3313_cartography/utils/defines.hpp>

#include <romhack_b3313_cartography/utils/rom_utils.h>
#include <romhack_b3313_cartography/utils/utilities.h>

#include <romhack_b3313_cartography/uis/Node.h>

#include "../threads/JsonLoaderThread.hpp"
#include "../threads/MainWindowUpdateThread.hpp"
#include "../uis/MouseFixGraphicScene.h"

#include <memory>
#include <romhack_b3313_cartography/utils/qt_includes.hpp>

#include "../utils/JsonLoading.h"
#include "SettingsWindow.h"
struct StarData {
    QString courseName;
    int numStars;
    bool collected = false;
    int offset;
    int mask;

    StarData() : numStars(0), collected(false), offset(0), mask(0) {}

    StarData(const QString &courseName, int numStars, bool collected, int offset, int mask)
        : courseName(courseName), numStars(numStars), collected(collected), offset(offset), mask(mask) {}
};
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
    static QPushButton *switchViewButton, *settingsButton;
    static QString jump_to_which_line;

    static QJsonObject lastJsonData;
    static QStringList courseNames, associatedCourses;
    static bool shiftPressed, showStarDisplay, force_toggle_star_display, jump_to_star_display_associated_line;

    static int startNodeIndex;
    static QVector<Node *> nodes;
    static QVector<QPair<int, int>> connections;
    static QGraphicsScene *graphicsScene;

  protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

  private slots:
    void changeNodeShape();
    void removeConnections();
    void saveNodes();
    void openSettingsWindow();

  private:
    void toggleStarDisplay();
    void textUpdate();
    void showDialog(const QString &labelText, QWidget *inputWidget, std::function<void()> onAccept);
    void closeEvent(QCloseEvent *event) override;
    bool isModified() const;
    void onTimerUpdate();
    void updateDisplay();
    void renameSelectedNode();
    void changeNodeColor();
    void associateStarToNode();
    void setWindowResizable(bool resizable);
    void displayStars(const QJsonObject &jsonData);
    void drawCourseStars(QPainter &painter,
                         const QMap<QString, QMap<QString, QVector<StarData>>> &groupCourseMap,
                         float startX, float starTextureHeight,
                         float rectLeft, float rectTop,
                         int &yOffset, int reservedHeight,
                         const QImage &starCollectedTexture,
                         const QImage &starMissingTexture,
                         const QStringList &associatedCourseNames,
                         QMap<QString, QRectF> &logoRects);
    void generateTabContent(const QString &tabName, const QPixmap &pixmap, QWidget *contentWidget, QVBoxLayout *contentLayout);
    QStringList getCourseNamesFromSlot0(const QJsonObject &jsonData);

    Node *findAssociatedNode();
    SettingsWindow *settingsWindow = nullptr;
    QPointF startPos;
    QList<Node *> mind_map_nodes;
    bool contextMenuOpened = false;
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
    QScrollArea *scrollArea;
    int stardisplayscrollPosition = 0, rightClickedNodeIndex = -1;
    QRectF groupTextRect;
    QMap<QString, QRectF> courseNameRects, logoRects;
    Node *nodeUnderCursor = nullptr;
    JsonLoaderThread *jsonLoaderThread = nullptr;
#ifdef DEBUG
    QString b33_13_mind_map_str = "b3313-v1.0.2-Mind_map.json";
#else
    QString b33_13_mind_map_str = "stars_layout/b3313-V1.0.2/b3313-v1.0.2-Mind_map.json";
#endif
};

#endif // MAIN_WINDOW_H