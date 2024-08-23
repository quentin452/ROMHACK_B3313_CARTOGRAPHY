#pragma once
#include <romhack_b3313_cartography/utils/qt_includes.hpp>

class JsonLoaderThread : public QThread {
    Q_OBJECT
  public:
    explicit JsonLoaderThread(QObject *parent = nullptr)
        : QThread(parent), running(true) {}

    void loadJson(const QString &filePath) {
        QMutexLocker locker(&mutex);
        this->filePath = filePath;
        filePathChanged = true;
    }

    void stop() {
        QMutexLocker locker(&mutex);
        running = false;
        condition.wakeOne();
    }

  signals:
    void jsonLoaded(const QJsonObject &jsonData);

  protected:
    void run() override {
        qDebug() << "JsonLoaderThread started.";
        while (true) {
            QMutexLocker locker(&mutex);
            while (!filePathChanged && running) {
                condition.wait(&mutex);
            }

            if (!running) {
                qDebug() << "JsonLoaderThread stopping.";
                break;
            }

            if (!filePath.isEmpty()) {
                QFile file(filePath);
                if (file.open(QIODevice::ReadOnly)) {
                    QByteArray jsonData = file.readAll();
                    file.close();
                    QJsonDocument document = QJsonDocument::fromJson(jsonData);
                    if (!document.isNull()) {
                        emit jsonLoaded(document.object());
                    } else {
                        qWarning() << "Failed to parse JSON";
                    }
                } else {
                    qWarning() << "Failed to open JSON file:" << filePath;
                }
                filePath.clear();
            }
            filePathChanged = false;
        }
    }

  private:
    QString filePath;
    bool running;
    bool filePathChanged = false;
    QMutex mutex;
    QWaitCondition condition;
};