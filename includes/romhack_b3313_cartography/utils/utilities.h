#ifndef UTILITIES_H
#define UTILITIES_H
#include "qt_includes.hpp"
extern QString GLOBAL_STAR_DISPLAY_JSON_STR;
extern QMutex globalMutex;
template <typename Layout, typename... Widgets>
void addWidgets(Layout &layout, Widgets &&...widgets) {
    (layout.addWidget(std::forward<Widgets>(widgets)), ...);
}
void simulateKeyPress(Qt::Key key);
void simulateKeyRelease(Qt::Key key);
bool isShiftPressed();
#endif // UTILITIES_H