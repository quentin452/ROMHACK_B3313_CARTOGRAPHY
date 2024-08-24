#pragma once
#include <QGraphicsView>
#include <QResizeEvent>
#include <QScrollBar>

class CustomGraphicView : public QGraphicsView {
    Q_OBJECT

  public:
    CustomGraphicView(QWidget *parent = nullptr);

  protected:
};
