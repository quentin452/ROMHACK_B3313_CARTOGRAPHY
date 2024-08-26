#include "windows/MainWindow.h"

#include "MindMapDownloader.hpp"
#include <iostream>
#include <romhack_b3313_cartography/utils/qt_includes.hpp>
#include <romhack_b3313_cartography/utils/utilities.h>
static QtMessageHandler defaultMessageHandler = nullptr;
void createNecessaryDirs() {
    QDir().mkpath(LAST_COMMIT_LOCAL_DIR);
    QDir().mkpath(UNOFFICIAL_MIND_MAP_LOCAL_DIR);
    QDir().mkpath(UNOFFICIAL_STARS_LAYOUT_LOCAL_DIR);
    QDir().mkpath(OFFICIAL_MIND_MAP_LOCAL_DIR);
    QDir().mkpath(OFFICIAL_STARS_LAYOUT_LOCAL_DIR);
}

QString getLastCommitDate() {
    QFile file(LAST_COMMIT_LOCAL_FILE);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

    QTextStream in(&file);
    return in.readLine().trimmed();
}
void saveLastCommitDate(const QString &commitDate) {
    QFile file(LAST_COMMIT_LOCAL_FILE);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << commitDate;
    }
}
void fetchLastCommitDateOnline(std::function<void(const QString &)> callback) {
    QUrl url("https://api.github.com/repos/quentin452/Mind-Map-Repo/commits?path=Mind-Maps&per_page=1");
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "ROMHACK_B3313_CARTOGRAPHY/V0.1");

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkReply *reply = manager->get(request);

    QObject::connect(reply, &QNetworkReply::finished, [reply, callback]() {
        QByteArray data = reply->readAll();
        QString commitDate;

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
            if (!jsonDoc.isNull() && jsonDoc.isArray()) {
                QJsonArray jsonArray = jsonDoc.array();
                if (!jsonArray.isEmpty()) {
                    QJsonObject commitObj = jsonArray.first().toObject();
                    commitDate = commitObj["commit"].toObject()["committer"].toObject()["date"].toString();
                } else {
                    qWarning() << "No commits found.";
                }
            } else {
                qWarning() << "Invalid JSON response.";
            }
        } else {
            qWarning() << "Network error:" << reply->errorString();
        }

        callback(commitDate);
        reply->deleteLater();
    });
}

QStringList findJsonFilesRecursively(const QString &directoryPath) {
    QStringList jsonFiles;
    QDir dir(directoryPath);
    if (!dir.exists())
        return jsonFiles;

    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    for (const QFileInfo &fileInfo : fileList) {
        if (fileInfo.suffix().toLower() == "json")
            jsonFiles << fileInfo.absoluteFilePath();
    }

    QFileInfoList dirList = dir.entryInfoList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    for (const QFileInfo &dirInfo : dirList) {
        jsonFiles << findJsonFilesRecursively(dirInfo.absoluteFilePath());
    }
    return jsonFiles;
}
void checkAndDownloadMindMaps(MindMapDownloader &downloader) {
    QString lastCommitDate = downloader.getLastCommitDate();

    fetchLastCommitDateOnline([&downloader, lastCommitDate](const QString &onlineCommitDate) {
        QDir mindMapDir(OFFICIAL_MIND_MAP_LOCAL_DIR);
        QStringList mindMapFiles = mindMapDir.entryList(QStringList() << "*.json", QDir::Files);

        if (lastCommitDate.isEmpty() || onlineCommitDate > lastCommitDate) {
            downloader.downloadMindMaps();
            downloader.saveLastCommitDate(onlineCommitDate);
              SHOW_MESSAGE("Official Mind Maps got Updated");
        } else if (mindMapFiles.isEmpty()) {
            downloader.downloadMindMaps();
              SHOW_MESSAGE("Official Mind Maps got Redownloaded because Official folder get emptied");
        } });
}

void myMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if (msg.contains("Unable to set geometry"))
        return;
    if (defaultMessageHandler)
        defaultMessageHandler(type, context, msg);
}
#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
bool isInternetAvailable() {
    return InternetCheckConnection(L"http://www.google.com", FLAG_ICC_FORCE_CONNECTION, 0);
}
#elif __linux__
#include <unistd.h>
bool isInternetAvailable() {
    return system("ping -c 1 google.com") == 0;
}
#elif __APPLE__
#include <unistd.h>
bool isInternetAvailable() {
    return system("ping -c 1 google.com") == 0;
}
#endif
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MindMapDownloader downloader;
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    WIDTH = screenWidth / 1.35;
    HEIGHT = screenHeight / 1.35;

    defaultMessageHandler = qInstallMessageHandler(myMessageHandler);
    createNecessaryDirs();

    if (!isInternetAvailable()) {
        QMessageBox errorMsg;
        errorMsg.setIcon(QMessageBox::Warning);
        errorMsg.setWindowTitle("No Internet Connection");
        errorMsg.setText("Attention you are not connected to internet, the software cannot download mind maps from internet. Do you want to continue?");
        errorMsg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        errorMsg.setDefaultButton(QMessageBox::No);
        errorMsg.setToolTip("Attention you are not connected to internet, the software cannot download mind maps from internet. Do you want to continue?");
        if (errorMsg.exec() == QMessageBox::No)
            return 0;
    }
    checkAndDownloadMindMaps(downloader);

    QStringList backends = {"d3d11", "opengl", "vulkan"};
    QInputDialog backendDialog;
    backendDialog.setOptions(QInputDialog::UseListViewForComboBoxItems);
    backendDialog.setWindowTitle("Choose Rendering API Backend");
    backendDialog.setLabelText("Backend:");
    backendDialog.setComboBoxItems(backends);
    backendDialog.setMinimumSize(300, 200);
    backendDialog.setMaximumSize(300, 200);

    if (backendDialog.exec() != QDialog::Accepted || backendDialog.textValue().isEmpty())
        return 0;

    QString backend = backendDialog.textValue();
    qputenv("QSG_RHI_BACKEND", backend.toUtf8());

    QStringList officialJsonFiles = findJsonFilesRecursively(OFFICIAL_STARS_LAYOUT_LOCAL_DIR);
    QStringList unofficialJsonFiles = findJsonFilesRecursively(UNOFFICIAL_STARS_LAYOUT_LOCAL_DIR);
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
    jsonDialog.setMinimumSize(WIDTH, HEIGHT);
    jsonDialog.setMaximumSize(WIDTH, HEIGHT);

    if (jsonDialog.exec() != QDialog::Accepted || jsonDialog.textValue().isEmpty())
        return 0;

    QString selectedStarLayoutFile = jsonDialog.textValue();
    GLOBAL_STAR_DISPLAY_JSON_STR = selectedStarLayoutFile.mid(selectedStarLayoutFile.indexOf(": ") + 2);

    QStringList officialmindMapJsonFiles = findJsonFilesRecursively(OFFICIAL_MIND_MAP_LOCAL_DIR);
    QStringList unofficialmindMapJsonFiles = findJsonFilesRecursively(UNOFFICIAL_MIND_MAP_LOCAL_DIR);
    QStringList jsonFileOptions2;
    for (const QString &file : officialmindMapJsonFiles) {
        jsonFileOptions2 << "Official: " + file;
    }
    for (const QString &file : unofficialmindMapJsonFiles) {
        jsonFileOptions2 << "Unofficial: " + file;
    }
    QInputDialog mindMapDialog;
    mindMapDialog.setOptions(QInputDialog::UseListViewForComboBoxItems);
    mindMapDialog.setWindowTitle("Choose Mind Map");
    mindMapDialog.setLabelText("Select a Mind Map JSON file:");
    mindMapDialog.setComboBoxItems(jsonFileOptions2);
    mindMapDialog.setMinimumSize(WIDTH, HEIGHT);
    mindMapDialog.setMaximumSize(WIDTH, HEIGHT);

    QPushButton *newMindMapButton = new QPushButton("Create New Mind Map", &mindMapDialog);
    mindMapDialog.layout()->addWidget(newMindMapButton);
    QObject::connect(newMindMapButton, &QPushButton::clicked, [&]() {
        bool ok;
        QString newMindMapName = QInputDialog::getText(nullptr, "Create New Mind Map", "Enter the name of the new mind map:", QLineEdit::Normal, "", &ok);
        if (ok && !newMindMapName.isEmpty()) {
            QString newMindMapFile = UNOFFICIAL_MIND_MAP_LOCAL_DIR + "/" + newMindMapName + ".json";
            if (QFile::exists(newMindMapFile)) {
                QMessageBox::warning(nullptr, "File Exists", "A mind map with this name already exists. Please choose a different name.");
            } else {
                QFile file(newMindMapFile);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write("{}");
                    file.close();
                    jsonFileOptions2 << "Unofficial: " + newMindMapFile;
                    mindMapDialog.setComboBoxItems(jsonFileOptions2);
                }
            }
        }
    });

    if (mindMapDialog.exec() != QDialog::Accepted || mindMapDialog.textValue().isEmpty())
        return 0;

    QString selectedMindMapFile = mindMapDialog.textValue();
    if (selectedMindMapFile.startsWith("Official: "))
        selectedMindMapFile = selectedMindMapFile.mid(QString("Official: ").length());
    else if (selectedMindMapFile.startsWith("Unofficial: "))
        selectedMindMapFile = selectedMindMapFile.mid(QString("Unofficial: ").length());

    GLOBAL_MIND_MAP_JSON_STR = selectedMindMapFile;
    MainWindow window;
    window.show();
    return app.exec();
}