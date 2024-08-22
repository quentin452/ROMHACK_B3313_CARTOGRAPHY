#include "windows/MainWindow.h"

#include <QApplication>
#include <QInputDialog>
#include <QStringList>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QStringList backends;
    backends << "d3d11"
             << "opengl"
             << "vulkan";
    QInputDialog dialog;
    dialog.setOptions(QInputDialog::UseListViewForComboBoxItems);
    dialog.setWindowTitle("Choose Rendering API Backend");
    dialog.setLabelText("Backend:");
    dialog.setComboBoxItems(backends);
    dialog.setMinimumSize(300, 200);
    dialog.setMaximumSize(300, 200);
    if (dialog.exec() == QDialog::Accepted) {
        QString backend = dialog.textValue();
        if (!backend.isEmpty()) {
            qputenv("QSG_RHI_BACKEND", backend.toUtf8());
            MainWindow window;
            window.show();
            return app.exec();
        }
    }
    return 0;
}