#pragma once
#include <romhack_b3313_cartography/utils/qt_includes.hpp>

class ProgressDialog : public QDialog {
    Q_OBJECT

  public:
    explicit ProgressDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Download Progress");
        QVBoxLayout *layout = new QVBoxLayout(this);

        progressBar = new QProgressBar(this);
        layout->addWidget(progressBar);

        QPushButton *cancelButton = new QPushButton("Cancel", this);
        layout->addWidget(cancelButton);
        connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

        QPushButton *acceptButton = new QPushButton("Accept", this);
        layout->addWidget(acceptButton);
        connect(acceptButton, &QPushButton::clicked, this, &QDialog::accept);

        setLayout(layout);
    }

    void setMaximum(int max) {
        progressBar->setMaximum(max);
    }

    void setValue(int value) {
        progressBar->setValue(value);
    }

    int value() const {
        return progressBar->value();
    }

    int maximum() const {
        return progressBar->maximum();
    }

  private:
    QProgressBar *progressBar;
};
