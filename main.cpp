#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  // Global stylesheet to visualize the layout structure
  // Frames will have borders so we can see the skeleton
  // Using dark theme consistent with the bottom panel
  a.setStyleSheet(
      "QFrame { border: 1px solid #444; background-color: #161616; }"
      "QSplitter::handle { background-color: #333; }"
      "QLabel { border: none; font-weight: bold; color: #ddd; }"
      "QMainWindow { background-color: #161616; }");

  MainWindow w;
  w.resize(1280, 720);
  w.show();

  return a.exec();
}
