#ifndef UTILITIES_H
#define UTILITIES_H
#include "qt_includes.hpp"
extern QString GLOBAL_STAR_DISPLAY_JSON_STR, GLOBAL_MIND_MAP_JSON_STR;
extern int WIDTH, HEIGHT;
extern const QString LAST_COMMIT_LOCAL_FILE, LAST_COMMIT_LOCAL_DIR, MIND_MAP_REPO_URL, UNOFFICIAL_MIND_MAP_LOCAL_DIR, UNOFFICIAL_STARS_LAYOUT_LOCAL_DIR, OFFICIAL_MIND_MAP_LOCAL_DIR, OFFICIAL_STARS_LAYOUT_LOCAL_DIR;
template <typename Layout, typename... Widgets>
void addWidgets(Layout &layout, Widgets &&...widgets) {
    (layout.addWidget(std::forward<Widgets>(widgets)), ...);
}
void simulateKeyPress(Qt::Key key);
void simulateKeyRelease(Qt::Key key);
bool isShiftPressed();
void showDialog(QWidget *parent, const QString &labelText, QWidget *inputWidget, std::function<void()> onAccept);
#endif // UTILITIES_H