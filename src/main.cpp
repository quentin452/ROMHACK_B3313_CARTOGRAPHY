#include "windows/MainWindow.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QInputDialog>
#include <QStringList>
#include <QTextStream>
#include <romhack_b3313_cartography/utils/utilities.h>

static QtMessageHandler defaultMessageHandler = nullptr;
void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if (msg.contains("Unable to set geometry"))
        return;
    if (defaultMessageHandler)
        defaultMessageHandler(type, context, msg);
}
QStringList findJsonFilesRecursively(const QString &directoryPath) {
    QStringList jsonFiles;
    QDir dir(directoryPath);
    if (!dir.exists()) {
        return jsonFiles;
    }

    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    for (const QFileInfo &fileInfo : fileList) {
        if (fileInfo.suffix().toLower() == "json") {
            jsonFiles << fileInfo.absoluteFilePath();
        }
    }

    QFileInfoList dirList = dir.entryInfoList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    for (const QFileInfo &dirInfo : dirList) {
        jsonFiles << findJsonFilesRecursively(dirInfo.absoluteFilePath());
    }

    return jsonFiles;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    defaultMessageHandler = qInstallMessageHandler(myMessageHandler);
    QStringList backends;
    backends << "d3d11"
             << "opengl"
             << "vulkan";
    QInputDialog backendDialog;
    backendDialog.setOptions(QInputDialog::UseListViewForComboBoxItems);
    backendDialog.setWindowTitle("Choose Rendering API Backend");
    backendDialog.setLabelText("Backend:");
    backendDialog.setComboBoxItems(backends);
    backendDialog.setMinimumSize(300, 200);
    backendDialog.setMaximumSize(300, 200);

    if (backendDialog.exec() == QDialog::Accepted) {
        QString backend = backendDialog.textValue();
        if (!backend.isEmpty()) {
            qputenv("QSG_RHI_BACKEND", backend.toUtf8());
            QString officialLayoutsDir = "resources/stars_layout";
            QString userDirectory = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            QString unofficialLayoutsDir = userDirectory + "/.ROMHACK_B3313_CARTOGRAPHY/UNOFFICIAL_STARS_LAYOUT";
            QDir().mkpath(unofficialLayoutsDir);
            QStringList officialJsonFiles = findJsonFilesRecursively(officialLayoutsDir);
            QStringList unofficialJsonFiles = findJsonFilesRecursively(unofficialLayoutsDir);
            QStringList jsonFileOptions;
            for (const QString &file : officialJsonFiles) {
                jsonFileOptions << "Official: " + file;
            }
            for (const QString &file : unofficialJsonFiles) {
                jsonFileOptions << "Unofficial: " + file;
            }
            QInputDialog jsonDialog;
            jsonDialog.setOptions(QInputDialog::UseListViewForComboBoxItems);
            jsonDialog.setWindowTitle("Choose Star Layout");
            jsonDialog.setLabelText("Select the Star Layout JSON file:");
            jsonDialog.setComboBoxItems(jsonFileOptions);
            jsonDialog.setMinimumSize(500, 300);
            jsonDialog.setMaximumSize(500, 300);
            if (jsonDialog.exec() == QDialog::Accepted) {
                QString selectedFile = jsonDialog.textValue();
                if (!selectedFile.isEmpty()) {
                    QString selectedFilePath = selectedFile.mid(selectedFile.indexOf(": ") + 2);
                    GLOBAL_STAR_DISPLAY_JSON_STR = selectedFilePath;
                    MainWindow window;
                    window.show();
                    return app.exec();
                }
            }
        }
    }
    return 0;
}