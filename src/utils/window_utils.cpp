#include <romhack_b3313_cartography/utils/defines.hpp>
#include <romhack_b3313_cartography/utils/window_utils.h>

bool WindowsUtils::isItemInScene(QGraphicsItem *item, QGraphicsScene *scene) {
    QList<QGraphicsItem *> items = scene->items();
    for (QGraphicsItem *existingItem : items) {
        QGraphicsTextItem *textItem = dynamic_cast<QGraphicsTextItem *>(existingItem);
        QGraphicsTextItem *inputTextItem = dynamic_cast<QGraphicsTextItem *>(item);
        if (textItem && inputTextItem && textItem->toPlainText() == inputTextItem->toPlainText()) {
            return true;
        }
    }
    return false;
}
