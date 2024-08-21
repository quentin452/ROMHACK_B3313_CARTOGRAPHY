#pragma once
#include <romhack_b3313_cartography/utils/qt_includes.hpp>

class MainWindowUpdateThread : public QThread {
    Q_OBJECT
  public:
    explicit MainWindowUpdateThread(QObject *parent = nullptr) : QThread(parent), running(true) {}

    void stop() {
        QMutexLocker locker(&mutex);
        running = false;
    }

  protected:
    void run() override {
        while (true) {
            {
                QMutexLocker locker(&mutex);
                if (!running) {
                    break;
                }
            }
            QThread::sleep(1); // Sleep for 1 second
            emit updateNeeded();
        }
    }

  signals:
    void updateNeeded();

  private:
    bool running;
    QMutex mutex;
};
