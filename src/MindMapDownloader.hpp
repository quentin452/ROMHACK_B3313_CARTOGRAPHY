#include "uis/dialogues/ProgressDialog.hpp"
#include <romhack_b3313_cartography/utils/qt_includes.hpp>
#include <romhack_b3313_cartography/utils/utilities.h>

class MindMapDownloader : public QObject {
    Q_OBJECT

  public:
    explicit MindMapDownloader(QObject *parent = nullptr)
        : QObject(parent), manager(new QNetworkAccessManager(this)) {
    }

    void downloadMindMaps() {
        QUrl url("https://api.github.com/repos/quentin452/Mind-Map-Repo/contents/Mind-Maps");
        QNetworkRequest request(url);
        request.setRawHeader("User-Agent", "ROMHACK_B3313_CARTOGRAPHY/V0.1");

        QNetworkReply *reply = manager->get(request);

        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QByteArray data = reply->readAll();

            if (reply->error() == QNetworkReply::NoError) {
                processInitialResponse(data);
            } else {
                qWarning() << "Initial request error:" << reply->errorString();
            }
            reply->deleteLater();
        });
    }

    QString getLastCommitDate() const {
        QFile file(LAST_COMMIT_LOCAL_FILE);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return QString();

        QTextStream in(&file);
        return in.readLine().trimmed();
    }

    void saveLastCommitDate(const QString &commitDate) const {
        QFile file(LAST_COMMIT_LOCAL_FILE);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << commitDate;
        } else {
            qWarning() << "Failed to open file for writing:" << file.errorString();
        }
    }

  private slots:
    void processInitialResponse(const QByteArray &data) {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

        if (!jsonDoc.isNull() && jsonDoc.isArray()) {
            QJsonArray jsonArray = jsonDoc.array();

            ProgressDialog *progressDialog = new ProgressDialog(); // Dynamically allocated
            progressDialog->setMaximum(jsonArray.size());
            progressDialog->show();

            int fileCount = 0;

            for (const QJsonValue &value : jsonArray) {
                if (value.isObject()) {
                    QJsonObject obj = value.toObject();
                    QString downloadUrl = obj["download_url"].toString();
                    QString fileName = obj["name"].toString();

                    if (fileName.endsWith(".json")) {
                        downloadFile(downloadUrl, fileName, progressDialog, fileCount);
                        fileCount++;
                    } else {
                        qDebug() << "Skipping non-JSON file:" << fileName;
                    }
                } else {
                    qWarning() << "Expected JSON object but got:" << value;
                }
            }

            // Wait until the progress dialog is closed
            connect(this, &MindMapDownloader::downloadsFinished, progressDialog, &QDialog::accept);

        } else {
            qWarning() << "Expected JSON array but got:" << jsonDoc.toJson();
        }
    }

    void downloadFile(const QString &url, const QString &fileName, ProgressDialog *progressDialog, int fileCount) {
        QUrl qurl(url);
        QNetworkRequest request(qurl);

        QNetworkReply *downloadReply = manager->get(request);
        clearDirectory(OFFICIAL_MIND_MAP_LOCAL_DIR);
        connect(downloadReply, &QNetworkReply::finished, this, [this, downloadReply, fileName, progressDialog, fileCount]() {
            QByteArray data = downloadReply->readAll();

            if (data.isEmpty()) {
                qWarning() << "Downloaded file is empty";
                return;
            }

            QFile file(OFFICIAL_MIND_MAP_LOCAL_DIR + "/" + fileName);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(data);
                file.close();
                qInfo() << "File downloaded and saved:" << file.fileName();
            } else {
                qWarning() << "Failed to save file:" << file.errorString();
            }

            progressDialog->setValue(fileCount + 1);

            if (progressDialog->value() == progressDialog->maximum()) {
                emit downloadsFinished();
            }

            downloadReply->deleteLater();
        });

        connect(downloadReply, &QNetworkReply::errorOccurred, this, [this](QNetworkReply::NetworkError code) {
            qWarning() << "File download network error:" << code;
        });

        connect(downloadReply, &QNetworkReply::sslErrors, this, [this](const QList<QSslError> &errors) {
            qWarning() << "File download SSL errors:" << errors;
        });
    }
    void clearDirectory(const QString &path) {
        QDir dir(path);
        if (dir.exists()) {
            dir.setFilter(QDir::Files);
            foreach (QString file, dir.entryList()) {
                dir.remove(file);
            }
        }
    }

  signals:
    void downloadsFinished();

  private:
    QNetworkAccessManager *manager;
};