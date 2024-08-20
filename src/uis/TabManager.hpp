#pragma once
#include "Tab.hpp"
#include <QFont>
#include <QMouseEvent>
#include <QPainter>
#include <QRectF>
#include <QString>
#include <QVector>
#include <QWidget>
#include <string>
#include <vector>

class TabManager : public QWidget {
    Q_OBJECT

  public:
    TabManager(const QStringList &tabNames, QWidget *parent = nullptr)
        : QWidget(parent), currentTabIndex(0) {
        initializeTabs(tabNames);
    }

    void initializeTabs(const QStringList &tabNames) {
        tabs.clear();
        float xOffset = 100;
        float yOffset = 50;
        for (const auto &name : tabNames) {
            tabs.emplace_back(name, xOffset, yOffset);
            xOffset += 110;
        }
        update(); // Request a repaint to show the tabs
    }

  protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        // Use int for indices to match the QVector type
        for (int i = 0; i < tabs.size(); ++i) {
            tabs[i].draw(painter, font(), i == currentTabIndex);
        }
    }

  public:
    void handleMouseClick(const QPointF &mousePos) {
        for (int i = 0; i < tabs.size(); ++i) {
            if (tabs[i].contains(mousePos)) {
                currentTabIndex = i;
                update(); // Request a repaint to show the selected tab
                return;
            }
        }
    }
    QString getCurrentTabName() const {
        return tabs[currentTabIndex].getName();
    }

    float getTabsHeight() const {
        return 30; // Height of the tabs, adjust if needed
    }
    bool contains(const QPointF &point) const {
        for (const auto &tab : tabs) {
            if (tab.contains(point)) {
                return true;
            }
        }
        return false;
    }

  private:
    QVector<Tab> tabs;
    int currentTabIndex; // Changed to int to match QVector index type
};