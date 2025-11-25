#include "MainWindow.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    // Global stylesheet to visualize the layout structure
    // Frames will have borders so wecan see the skeleton
    a.setStyleSheet(
        "QFrame { border: 1px solid #444; background-color: #f0f0f0; }"
        "QSplitter::handle { background-color: #aaa; }"
        "QLabel { border: none; font-weight: bold; color: #333; }"
    );

    MainWindow w;
    w.resize(1280, 720);
    w.show();

    return a.exec();
}
