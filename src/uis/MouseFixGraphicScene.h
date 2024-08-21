#pragma once
#include "../windows/MainWindow.h"
#include <romhack_b3313_cartography/utils/qt_includes.hpp>


class MouseFixGraphicScene : public QGraphicsScene {
    Q_OBJECT

  public:
    MouseFixGraphicScene(QObject *parent = nullptr) : QGraphicsScene(parent) {}

  protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};
