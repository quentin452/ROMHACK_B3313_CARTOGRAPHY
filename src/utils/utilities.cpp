#include <romhack_b3313_cartography/utils/utilities.h>
QString GLOBAL_STAR_DISPLAY_JSON_STR;
void simulateKeyPress(Qt::Key key) {
    QKeyEvent keyPressEvent(QEvent::KeyPress, key, Qt::NoModifier);
    QTest::keyClick(QApplication::focusWidget(), key, Qt::NoModifier);
}
void simulateKeyRelease(Qt::Key key) {
    QKeyEvent keyReleaseEvent(QEvent::KeyRelease, key, Qt::NoModifier);
    QTest::keyRelease(QApplication::focusWidget(), key, Qt::NoModifier);
}
bool isShiftPressed() {
    return QApplication::keyboardModifiers() & Qt::ShiftModifier;
}
bool isCtrlPressed() {
    return QApplication::keyboardModifiers() & Qt::ControlModifier;
}
void showDialog(QWidget *parent, const QString &labelText, QWidget *inputWidget, std::function<void()> onAccept) {
    QDialog dialog(parent);
    dialog.setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    dialog.setMinimumSize(200, 120);
    dialog.setMaximumSize(600, 120);
    QVBoxLayout layout(&dialog);
    QLabel label(labelText);
    QPushButton okButton(QObject::tr("OK"));
    QPushButton cancelButton(QObject::tr("Cancel"));
    addWidgets(layout, &label, inputWidget, &okButton, &cancelButton);
    QObject::connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(&cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);
    if (dialog.exec() == QDialog::Accepted)
        onAccept();
}