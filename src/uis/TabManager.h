#pragma once
#include <romhack_b3313_cartography/uis/Tab.hpp>
#include <romhack_b3313_cartography/utils/qt_includes.hpp>



class TabManager : public QWidget {
    Q_OBJECT

  public:
    TabManager(const QStringList &tabNames, QWidget *parent = nullptr);
    void initializeTabs(const QStringList &tabNames);

  protected:
    void paintEvent(QPaintEvent *event) override;

  public:
    void handleMouseClick(const QPointF &mousePos);
    QString getCurrentTabName() const;

    float getTabsHeight() const;
    bool contains(const QPointF &point) const;

  private:
    QVector<Tab> tabs;
    int currentTabIndex; // Changed to int to match QVector index type
};