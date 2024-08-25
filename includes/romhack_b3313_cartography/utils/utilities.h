#ifndef UTILITIES_H
#define UTILITIES_H
#include "qt_includes.hpp"
extern QString GLOBAL_STAR_DISPLAY_JSON_STR;
template <typename Layout, typename... Widgets>
void addWidgets(Layout &layout, Widgets &&...widgets) {
    (layout.addWidget(std::forward<Widgets>(widgets)), ...);
}
void simulateKeyPress(Qt::Key key);
void simulateKeyRelease(Qt::Key key);
bool isShiftPressed();
void showDialog(QWidget *parent, const QString &labelText, QWidget *inputWidget, std::function<void()> onAccept);
#endif // UTILITIES_H