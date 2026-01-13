#include "MainWindow.h"
#include "ChartWidget.h"
#include "TickerPlaceholder.h"
#include "TradingBottomPanel.h"
#include "orderbook.h"


#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) { setupUi(); }

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
  // Central widget to hold everything
  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);
  centralWidget->setStyleSheet("background-color: #121212; color: #ffffff;");

  // 1. MAIN LAYOUT
  QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(1);

  // --- LEFT CONTAINER (YELLOW BOX) ---
  QWidget *yellowContainer = new QWidget();
  QVBoxLayout *yellowLayout = new QVBoxLayout(yellowContainer);
  yellowLayout->setContentsMargins(0, 0, 0, 0);
  yellowLayout->setSpacing(1);

  // SUPPRIMÉ ICI : Le TickerPlaceholder n'est plus ajouté au mainLayout
  // TickerPlaceholder *tickerWidget = new TickerPlaceholder(this);
  // mainLayout->addWidget(tickerWidget);

  // --- TOP LEFT CONTAINER (GREEN BOX) ---
  QWidget *greenContainer = new QWidget();
  QVBoxLayout *greenLayout = new QVBoxLayout(greenContainer);
  greenLayout->setContentsMargins(0, 0, 0, 0);
  greenLayout->setSpacing(1);

  // ZONE 1: Pair Info (Top of Green)
  // QFrame* zone1 = new QFrame();
  // zone1->setFixedHeight(50);
  // zone1->setStyleSheet("background-color: #1e1e1e; border: 1px solid
  // #00ff00;"); QVBoxLayout* z1l = new QVBoxLayout(zone1); z1l->addWidget(new
  // QLabel("ZONE 1: PAIR INFO")); greenLayout->addWidget(zone1);

  // --- CHANGEMENT : AJOUT DU TICKER ICI (ENTRE ZONE 1 et ZONE 2) ---
  TickerPlaceholder *tickerWidget = new TickerPlaceholder(this);
  // On peut fixer une hauteur pour qu'il ressemble à une barre et ne prenne pas
  // trop de place tickerWidget->setFixedHeight(60); // Décommentez si vous
  // voulez une hauteur fixe
  greenLayout->addWidget(tickerWidget);
  // -----------------------------------------------------------------

  // ZONE 2: Chart (Bottom of Green)
  QFrame *zone2 = new QFrame();
  zone2->setStyleSheet("background-color: #1e1e1e;");
  QVBoxLayout *z2l = new QVBoxLayout(zone2);

  ChartWidget *chartWidget = new ChartWidget();
  z2l->addWidget(chartWidget);

  greenLayout->addWidget(zone2, 1); // Expands to fill remaining Green space

  // Container PINK
  QWidget *pinkContainer = new QWidget();
  // Note: Si vous voulez que le Carnet d'ordre (Zone 3) soit à DROITE du
  // graphique, changez QVBoxLayout en QHBoxLayout ici :
  QHBoxLayout *pinkLayout = new QHBoxLayout(pinkContainer);
  pinkLayout->setContentsMargins(
      0, 0, 0, 0); // Bonnes pratiques pour éviter les marges doubles
  pinkLayout->setSpacing(1);

  pinkLayout->addWidget(greenContainer, 3); // Green takes 75% of Pink width

  // ZONE 3: Order Book
  QFrame *zone3 = new QFrame();
  zone3->setStyleSheet("background-color: #1e1e1e;");
  QVBoxLayout *z3l = new QVBoxLayout(zone3);
  OrderBook *orderBook = new OrderBook(zone3);
  z3l->addWidget(orderBook);
  pinkLayout->addWidget(zone3, 1); // Zone 3 takes 25% of Pink width

  yellowLayout->addWidget(pinkContainer, 3); // Pink takes 75% of Yellow height

  // ZONE 5: Open Orders
  QFrame *zone5 = new QFrame();
  zone5->setStyleSheet("background-color: #1e1e1e;");
  QVBoxLayout *z5l = new QVBoxLayout(zone5);
  TradingBottomPanel *bottomPanel = new TradingBottomPanel(zone5);
  z5l->addWidget(bottomPanel);
  yellowLayout->addWidget(zone5, 1);

  mainLayout->addWidget(yellowContainer,
                        1); // Yellow takes remaining width (stretch 1)

  // --- RIGHT CONTAINER (ZONE 4) ---
  QFrame *zone4 = new QFrame();
  zone4->setFixedWidth(300);
  zone4->setStyleSheet("background-color: #1e1e1e;");
  QVBoxLayout *z4l = new QVBoxLayout(zone4);
  z4l->addWidget(new QLabel("ZONE 4: ORDER ENTRY"));

  mainLayout->addWidget(zone4, 0);
}