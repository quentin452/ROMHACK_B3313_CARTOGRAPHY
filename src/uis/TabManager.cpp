#include "TabManager.h"

TabManager::TabManager(const QStringList &tabNames, QWidget *parent)
    : QWidget(parent), currentTabIndex(0) {
    initializeTabs(tabNames);
}

void TabManager::initializeTabs(const QStringList &tabNames) {
    tabs.clear();
    float xOffset = 100;
    float yOffset = 50;
    for (const auto &name : tabNames) {
        tabs.emplace_back(name, xOffset, yOffset);
        xOffset += 110;
    }
    update(); // Request a repaint to show the tabs
}

void TabManager::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    // Use int for indices to match the QVector type
    for (int i = 0; i < tabs.size(); ++i) {
        tabs[i].draw(painter, font(), i == currentTabIndex);
    }
}

void TabManager::handleMouseClick(const QPointF &mousePos) {
    for (int i = 0; i < tabs.size(); ++i) {
        if (tabs[i].contains(mousePos)) {
            currentTabIndex = i;
            update(); // Request a repaint to show the selected tab
            return;
        }
    }
}
QString TabManager::getCurrentTabName() const {
    return tabs[currentTabIndex].getName();
}

float TabManager::getTabsHeight() const {
    return 30; // Height of the tabs, adjust if needed
}
bool TabManager::contains(const QPointF &point) const {
    for (const auto &tab : tabs) {
        if (tab.contains(point)) {
            return true;
        }
    }
    return false;
}
#include "TabManager.moc"
