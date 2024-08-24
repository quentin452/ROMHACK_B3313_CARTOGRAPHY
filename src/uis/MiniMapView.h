#pragma once
#include <romhack_b3313_cartography/utils/qt_includes.hpp>
class MiniMapView : public QGraphicsView {
    Q_OBJECT

  public:
    MiniMapView(QGraphicsScene *scene, QWidget *parent = nullptr) : QGraphicsView(scene, parent) {}

  protected:
    void mousePressEvent(QMouseEvent *event) override;

  signals:
    void minimapClicked(QMouseEvent *event);
};