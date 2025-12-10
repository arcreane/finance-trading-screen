#include "MainWindow.h"
#include "ChartWidget.h"
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QComboBox>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) { setupUi(); }

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
  // Central widget to hold everything
  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);
  centralWidget->setStyleSheet("background-color: #121212; color: #ffffff;");

  // 1. MAIN LAYOUT (RED BOX in sketch) - Horizontal Split
  // Left Part (Yellow) vs Right Part (Zone 4)
  QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(1);

  // --- LEFT CONTAINER (YELLOW BOX) ---
  // Vertical Split: Top (Pink) vs Bottom (Zone 5)
  QWidget *yellowContainer = new QWidget();
  QVBoxLayout *yellowLayout = new QVBoxLayout(yellowContainer);
  yellowLayout->setContentsMargins(0, 0, 0, 0);
  yellowLayout->setSpacing(1);

  // --- TOP CONTAINER (PINK BOX) ---
  // Horizontal Split: Left (Green) vs Right (Zone 3)
  QWidget *pinkContainer = new QWidget();
  QHBoxLayout *pinkLayout = new QHBoxLayout(pinkContainer);
  pinkLayout->setContentsMargins(0, 0, 0, 0);
  pinkLayout->setSpacing(1);

  // --- TOP LEFT CONTAINER (GREEN BOX) ---
  // Vertical Split: Top (Zone 1) vs Bottom (Zone 2)
  QWidget *greenContainer = new QWidget();
  QVBoxLayout *greenLayout = new QVBoxLayout(greenContainer);
  greenLayout->setContentsMargins(0, 0, 0, 0);
  greenLayout->setSpacing(1);

  // ZONE 1: Pair Info (Top of Green)
  QFrame *zone1 = new QFrame();
  zone1->setFixedHeight(50); // Fixed height for header
  zone1->setStyleSheet(
      "background-color: #1e1e1e; border: 1px solid #00ff00;"); // Green border
  
  QHBoxLayout *z1l = new QHBoxLayout(zone1);
  z1l->setContentsMargins(10, 0, 10, 0);
  
  QLabel *tickerLabel = new QLabel("Ticker:");
  tickerLabel->setStyleSheet("color: #d1d4dc; border: none;");
  QComboBox *tickerCombo = new QComboBox();
  tickerCombo->addItems({"BTC", "ETH", "SOL"});
  tickerCombo->setStyleSheet("background-color: #2a2e39; color: white; border: none; padding: 5px;");
  
  QLabel *tfLabel = new QLabel("Timeframe:");
  tfLabel->setStyleSheet("color: #d1d4dc; border: none;");
  QComboBox *tfCombo = new QComboBox();
  tfCombo->addItems({"Daily", "4h", "1h", "15m"});
  tfCombo->setStyleSheet("background-color: #2a2e39; color: white; border: none; padding: 5px;");

  QPushButton *loadBtn = new QPushButton("Load");
  loadBtn->setStyleSheet("background-color: #2962ff; color: white; border: none; padding: 5px 15px; font-weight: bold;");
  
  z1l->addWidget(tickerLabel);
  z1l->addWidget(tickerCombo);
  z1l->addSpacing(20);
  z1l->addWidget(tfLabel);
  z1l->addWidget(tfCombo);
  z1l->addStretch();
  z1l->addWidget(loadBtn);
  
  greenLayout->addWidget(zone1);

  // ZONE 2: Chart (Bottom of Green)
  ChartWidget *chartWidget = new ChartWidget();
  greenLayout->addWidget(chartWidget, 1);
  
  // Connect Load Button
  connect(loadBtn, &QPushButton::clicked, [=]() {
      chartWidget->loadData(tickerCombo->currentText(), tfCombo->currentText());
  });

  pinkLayout->addWidget(greenContainer, 3); // Green takes 75% of Pink width

  // ZONE 3: Order Book (Right of Pink)
  QFrame *zone3 = new QFrame();
  zone3->setStyleSheet(
      "background-color: #1e1e1e; border: 1px solid #ff00ff;"); // Pink/Magenta
                                                                // border
  QVBoxLayout *z3l = new QVBoxLayout(zone3);
  z3l->addWidget(new QLabel("ZONE 3: ORDER BOOK"));
  pinkLayout->addWidget(zone3, 1); // Zone 3 takes 25% of Pink width

  yellowLayout->addWidget(pinkContainer, 3); // Pink takes 75% of Yellow height

  // ZONE 5: Open Orders (Bottom of Yellow)
  QFrame *zone5 = new QFrame();
  zone5->setStyleSheet(
      "background-color: #1e1e1e; border: 1px solid #ffff00;"); // Yellow border
  QVBoxLayout *z5l = new QVBoxLayout(zone5);
  z5l->addWidget(new QLabel("ZONE 5: OPEN ORDERS / POSITIONS"));
  yellowLayout->addWidget(zone5, 1); // Zone 5 takes 25% of Yellow height

  mainLayout->addWidget(yellowContainer, 4); // Yellow takes 80% of width

  // --- RIGHT CONTAINER (ZONE 4) ---
  // Order Entry
  QFrame *zone4 = new QFrame();
  zone4->setFixedWidth(300); // Fixed width or weight
  zone4->setStyleSheet(
      "background-color: #1e1e1e; border: 1px solid #0000ff;"); // Blue border
  QVBoxLayout *z4l = new QVBoxLayout(zone4);
  z4l->addWidget(new QLabel("ZONE 4: ORDER ENTRY"));

  mainLayout->addWidget(
      zone4, 0); // Fixed width, so stretch 0 or 1 doesn't matter much if fixed
                 // width is set, but let's rely on fixed width.
}
