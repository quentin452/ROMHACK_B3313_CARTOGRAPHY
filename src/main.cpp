#include "windows/MainWindow.h"

#include <romhack_b3313_cartography/uis/Textures.h>
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Textures textures;
    MainWindow window;
    window.show();
    return app.exec();
}
