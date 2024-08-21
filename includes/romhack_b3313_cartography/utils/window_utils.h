#ifndef WINDOWS_UTILS_H
#define WINDOWS_UTILS_H

#include <QGraphicsItem>
#include <QGraphicsScene>

class WindowsUtils {
  public:
    static bool isItemInScene(QGraphicsItem *item, QGraphicsScene *scene);
};

#endif // WINDOWS_UTILS_H