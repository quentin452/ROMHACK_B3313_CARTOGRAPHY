#include <romhack_b3313_cartography/utils/utilities.h>
QString GLOBAL_STAR_DISPLAY_JSON_STR;
QMutex globalMutex;
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