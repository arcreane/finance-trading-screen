/**
 * @file main.cpp
 * @brief Application entry point for the Trading Screen application.
 * 
 * Initializes the Qt application, applies the global dark theme stylesheet,
 * and displays the main window.
 */

#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  // Apply global dark theme stylesheet
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
