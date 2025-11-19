#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFrame>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    // Central widget to hold everything
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Main Vertical Layout (Top Bars + Workspace)
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(2);

    // 1. Top Menu Bar Placeholder
    QFrame *topMenuBar = new QFrame();
    topMenuBar->setFixedHeight(40);
    QHBoxLayout *menuLayout = new QHBoxLayout(topMenuBar);
    menuLayout->addWidget(new QLabel("MENU | NEW | TOOLS | WORKSPACES | CONNECTIONS API | HELP"));
    mainLayout->addWidget(topMenuBar);

    // 2. Secondary Toolbar / Ticker Placeholder
    QFrame *tickerBar = new QFrame();
    tickerBar->setFixedHeight(50);
    QHBoxLayout *tickerLayout = new QHBoxLayout(tickerBar);
    tickerLayout->addWidget(new QLabel("ETH/USDT | DAILY | INDICATORS | Bloomberg News Ticker..."));
    mainLayout->addWidget(tickerBar);

    // 3. Main Workspace Area (Horizontal Splitter)
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    mainLayout->addWidget(mainSplitter);

    // --- LEFT COLUMN (Charts & Order Entry) ---
    QSplitter *leftSplitter = new QSplitter(Qt::Vertical);
    mainSplitter->addWidget(leftSplitter);

    // Chart Area
    QFrame *chartFrame = new QFrame();
    chartFrame->setMinimumHeight(200);
    QVBoxLayout *chartLayout = new QVBoxLayout(chartFrame);
    chartLayout->addWidget(new QLabel("MAIN CHART AREA (Candlesticks)"));
    leftSplitter->addWidget(chartFrame);

    // Indicator Area
    QFrame *indicatorFrame = new QFrame();
    indicatorFrame->setMinimumHeight(100);
    QVBoxLayout *indicatorLayout = new QVBoxLayout(indicatorFrame);
    indicatorLayout->addWidget(new QLabel("INDICATOR AREA (RSI, MACD, etc.)"));
    leftSplitter->addWidget(indicatorFrame);

    // Bottom Panel (Order Entry & Trade Table)
    QFrame *bottomPanel = new QFrame();
    bottomPanel->setMaximumHeight(150);
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomPanel);
    
    QFrame *orderEntryBlock = new QFrame(); // "BUY / AMOUNT / PNL"
    QVBoxLayout *ol = new QVBoxLayout(orderEntryBlock);
    ol->addWidget(new QLabel("ORDER ENTRY CONTROLS"));
    
    QFrame *tradeTableBlock = new QFrame(); // "Side PRICE TYPE..."
    QVBoxLayout *tl = new QVBoxLayout(tradeTableBlock);
    tl->addWidget(new QLabel("OPEN ORDERS / TRADES"));

    QFrame *consoleBlock = new QFrame(); // "Console > ..."
    QVBoxLayout *cl = new QVBoxLayout(consoleBlock);
    cl->addWidget(new QLabel("CONSOLE OUTPUT"));

    bottomLayout->addWidget(orderEntryBlock, 1);
    bottomLayout->addWidget(tradeTableBlock, 2);
    bottomLayout->addWidget(consoleBlock, 2);
    
    leftSplitter->addWidget(bottomPanel);
    
    // Set stretch factors for left column (Chart gets most space)
    leftSplitter->setStretchFactor(0, 3);
    leftSplitter->setStretchFactor(1, 1);
    leftSplitter->setStretchFactor(2, 0);


    // --- RIGHT COLUMN (Order Book, Watchlist, Depth) ---
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical);
    mainSplitter->addWidget(rightSplitter);

    // Top Right Section: Split horizontally for Order Book & Watchlist
    QSplitter *topRightSplitter = new QSplitter(Qt::Horizontal);
    
    QFrame *orderBookFrame = new QFrame();
    QVBoxLayout *obl = new QVBoxLayout(orderBookFrame);
    obl->addWidget(new QLabel("ORDER BOOK\n(Bids/Asks)"));
    
    QFrame *watchlistFrame = new QFrame();
    QVBoxLayout *wll = new QVBoxLayout(watchlistFrame);
    wll->addWidget(new QLabel("WATCHLIST"));

    topRightSplitter->addWidget(orderBookFrame);
    topRightSplitter->addWidget(watchlistFrame);
    rightSplitter->addWidget(topRightSplitter);

    // Middle Right Section: Depth / Details
    QFrame *depthFrame = new QFrame();
    QVBoxLayout *dl = new QVBoxLayout(depthFrame);
    dl->addWidget(new QLabel("MARKET DEPTH / DETAILS"));
    rightSplitter->addWidget(depthFrame);

    // Bottom Right Section: Trading Actions
    QFrame *actionFrame = new QFrame();
    QVBoxLayout *al = new QVBoxLayout(actionFrame);
    al->addWidget(new QLabel("TRADING ACTIONS\n(Limit/Market/Buy/Sell Buttons)"));
    rightSplitter->addWidget(actionFrame);

    // Set stretch factors for right column
    rightSplitter->setStretchFactor(0, 2);
    rightSplitter->setStretchFactor(1, 2);
    rightSplitter->setStretchFactor(2, 1);

    // Set main splitter stretch (Left column wider)
    mainSplitter->setStretchFactor(0, 3);
    mainSplitter->setStretchFactor(1, 1);
}
