#ifndef SETTINGS_WINDOW_H
#define SETTINGS_WINDOW_H 
#include <romhack_b3313_cartography/utils/qt_includes.hpp>
#include "MainWindow.h"
class SettingsWindow : public QDialog {
    Q_OBJECT
  public:
    SettingsWindow(QWidget *parent = nullptr);
    bool vsyncEnabled() const;
    bool showFpsEnabled() const;

  private slots:
    void onVsyncToggled(bool checked);
    void onShowFpsToggled(bool checked);

  private:
    QCheckBox *vsyncCheckBox;
    QCheckBox *showFpsCheckBox;
};
#endif // SETTINGS_WINDOW_H