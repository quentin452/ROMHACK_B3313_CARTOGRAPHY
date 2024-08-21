#pragma once
#include "../windows/MainWindow.h"
#include <romhack_b3313_cartography/utils/qt_includes.hpp>

class MouseFixGraphicScene : public QGraphicsScene {
    Q_OBJECT

  public:
    explicit MouseFixGraphicScene(QObject *parent = nullptr) : QGraphicsScene(parent) {}

  private:
    bool isMouseOverNode(const QPointF &mousePos, int &nodeIndex);

  protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};
