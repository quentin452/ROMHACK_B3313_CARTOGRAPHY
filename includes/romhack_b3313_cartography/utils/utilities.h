#pragma once
#include "qt_includes.hpp"
template <typename Layout, typename... Widgets>
void addWidgets(Layout &layout, Widgets &&...widgets) {
    (layout.addWidget(std::forward<Widgets>(widgets)), ...);
}

void simulateKeyPress(Qt::Key key);
void simulateKeyRelease(Qt::Key key);
bool isShiftPressed();