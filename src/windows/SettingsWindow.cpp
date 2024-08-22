#include "SettingsWindow.h"

SettingsWindow::SettingsWindow(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Settings");

    QVBoxLayout *layout = new QVBoxLayout(this);

    resizableCheckBox = new QCheckBox("Resizable Window", this); // Nouveau QCheckBox
    layout->addWidget(resizableCheckBox); // Ajouter au layout
    QPushButton *closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    layout->addWidget(closeButton);
    setLayout(layout);

    setMinimumSize(200, 100);
}

bool SettingsWindow::isResizable() const { 
    return resizableCheckBox->isChecked();
}
#include "SettingsWindow.moc"
