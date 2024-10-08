#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H
#include "MainWindow.h"
#include <romhack_b3313_cartography/utils/qt_includes.hpp>

class SettingsWindow : public QDialog {
    Q_OBJECT
  public:
    SettingsWindow(QWidget *parent = nullptr);
    bool isResizable() const;
  private:
    QCheckBox *vsyncCheckBox,*resizableCheckBox,*showFpsCheckBox;
};
#endif // SETTINGS_WINDOW_H