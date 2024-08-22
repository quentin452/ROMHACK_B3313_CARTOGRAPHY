#include "SettingsWindow.h"

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Settings");

    QVBoxLayout *layout = new QVBoxLayout(this);

    vsyncCheckBox = new QCheckBox("Enable V-Sync", this);
    showFpsCheckBox = new QCheckBox("Show FPS", this);

    layout->addWidget(vsyncCheckBox);
    layout->addWidget(showFpsCheckBox);

    QPushButton *closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    layout->addWidget(closeButton);
    setLayout(layout);

    setMinimumSize(200, 100);
}

bool SettingsWindow::vsyncEnabled() const {
    return vsyncCheckBox->isChecked();
}

bool SettingsWindow::showFpsEnabled() const {
    return showFpsCheckBox->isChecked();
}

void SettingsWindow::onVsyncToggled(bool checked) {return;
    QOpenGLWidget *glWidget = findChild<QOpenGLWidget *>();
    if (glWidget) {
        QSurfaceFormat format = glWidget->format();
        format.setSwapInterval(checked ? 1 : 0); // 1 pour activer la V-Sync, 0 pour désactiver
        glWidget->setFormat(format);
        glWidget->update();
    }
}

void SettingsWindow::onShowFpsToggled(bool checked) {return;
    if (MainWindow *mainWindow = qobject_cast<MainWindow *>(parent())) {
        if (checked) {
          //  mainWindow->fpsLabel->show();
        } else {
          //  mainWindow->fpsLabel->hide();
        }
    }
}
#include "SettingsWindow.moc"
