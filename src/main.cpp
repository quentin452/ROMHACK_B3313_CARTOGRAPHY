#include "windows/MainWindow.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QInputDialog>
#include <QStringList>
#include <QTextStream>
static QtMessageHandler defaultMessageHandler = nullptr;
void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if (msg.contains("Unable to set geometry")) 
        return;
    if (defaultMessageHandler) 
        defaultMessageHandler(type, context, msg);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    defaultMessageHandler = qInstallMessageHandler(myMessageHandler);
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
