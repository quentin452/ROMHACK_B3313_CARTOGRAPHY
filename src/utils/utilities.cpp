#include <romhack_b3313_cartography/utils/utilities.h>

void simulateKeyPress(Qt::Key key) {
    QKeyEvent keyPressEvent(QEvent::KeyPress, key, Qt::NoModifier);
    QTest::keyClick(QApplication::focusWidget(), key, Qt::NoModifier);
}

void simulateKeyRelease(Qt::Key key) {
    QKeyEvent keyReleaseEvent(QEvent::KeyRelease, key, Qt::NoModifier);
    QTest::keyRelease(QApplication::focusWidget(), key, Qt::NoModifier);
}
