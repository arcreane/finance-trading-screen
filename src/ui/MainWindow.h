/**
 * @file MainWindow.h
 * @brief Main application window for the Trading Screen.
 * 
 * Contains the main layout structure with chart widget, order book,
 * ticker selector, order entry panel, and trading bottom panel.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

/**
 * @class MainWindow
 * @brief Central window managing the trading application layout.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void setupUi();
};

#endif // MAINWINDOW_H
