#include "TradingBottomPanel.h"
#include <QBrush>
#include <QColor>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QPalette>
#include <QVBoxLayout>


TradingBottomPanel::TradingBottomPanel(QWidget *parent) : QTabWidget(parent) {
  setupTabs();
  setupStyle();
}

void TradingBottomPanel::setupTabs() {
  addTab(createPositionsTab(), "Positions (0)");
  addTab(createOpenOrdersTab(), "Open orders (0)");
  addTab(createOrderHistoryTab(), "Order history");
  addTab(createTradeHistoryTab(), "Trade history");
  addTab(createTransactionHistoryTab(), "Transaction history");
  addTab(createDepositsWithdrawalsTab(), "Deposits & withdrawals");
  addTab(createAssetsTab(), "Assets");
}

void TradingBottomPanel::setupStyle() {
  // Dark theme - comprehensive background fix
  this->setStyleSheet(
      "QWidget { background-color: #161616; color: #ddd; font-family: 'Segoe "
      "UI', sans-serif; }"
      "QTabWidget { background-color: #161616; border: none; }"
      "QTabWidget::pane { background-color: #161616; border-top: 1px solid "
      "#333; margin: 0px; padding: 0px; }"
      "QTabBar { background-color: #161616; }"
      "QTabBar::tab { background: #161616; color: #888; padding: 10px 15px; "
      "border: none; font-weight: bold; }"
      "QTabBar::tab:selected { color: #fff; border-bottom: 2px solid #fff; }"
      "QTabBar::tab:hover { color: #ccc; }"
      "QTableWidget { background-color: #161616; border: none; gridline-color: "
      "#161616; }"
      "QTableView { background-color: #161616; selection-background-color: "
      "#222; }"
      "QHeaderView::section { background-color: #161616; color: #666; border: "
      "none; padding: 5px; font-weight: normal; }"
      "QScrollBar:vertical { border: none; background: #161616; width: 8px; "
      "margin: 0px; }"
      "QScrollBar::handle:vertical { background: #333; min-height: 20px; "
      "border-radius: 4px; }"
      "QScrollBar::add-line:vertical { height: 0px; }"
      "QScrollBar::sub-line:vertical { height: 0px; }"
      "QLabel { background-color: #161616; color: #ddd; }"
      "QFrame { background-color: #161616; }");

  // Also set palette for thoroughness
  QPalette pal = this->palette();
  pal.setColor(QPalette::Window, QColor("#161616"));
  pal.setColor(QPalette::Base, QColor("#161616"));
  pal.setColor(QPalette::AlternateBase, QColor("#161616"));
  pal.setColor(QPalette::Button, QColor("#161616"));
  this->setPalette(pal);
  this->setAutoFillBackground(true);
  this->setAttribute(Qt::WA_StyledBackground, true);
}

QTableWidget *TradingBottomPanel::createTable(const QStringList &headers) {
  QTableWidget *table = new QTableWidget();
  table->setColumnCount(headers.size());
  table->setHorizontalHeaderLabels(headers);

  // Table Styling
  table->setShowGrid(false);
  table->verticalHeader()->setVisible(false);
  table->setFrameShape(QFrame::NoFrame);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);

  // Ensure viewport is dark
  table->viewport()->setStyleSheet("background-color: #161616;");
  table->viewport()->setAutoFillBackground(true);

  // Header Styling
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft |
                                                 Qt::AlignVCenter);
  table->horizontalHeader()->setStyleSheet(
      "QHeaderView::section { background-color: #161616; color: #666; border: "
      "none; padding: 5px; font-weight: normal; }");

  table->setStyleSheet(
      "QTableWidget { background-color: #161616; border: none; }"
      "QTableView { background-color: #161616; }"
      "QTableWidget::item { padding: 5px; border-bottom: 1px solid #222; }");

  return table;
}

QWidget *TradingBottomPanel::createPositionsTab() {
  QStringList headers = {
      "Symbol",     "Size",       "Entry price",        "Mark price", "Margin",
      "Liq. price", "PNL (ROE%)", "TP/SL for position", "Reverse"};
  QTableWidget *table = createTable(headers);

  QWidget *container = new QWidget();
  container->setStyleSheet("background-color: #161616;");
  container->setAutoFillBackground(true);
  QPalette pal = container->palette();
  pal.setColor(QPalette::Window, QColor("#161616"));
  container->setPalette(pal);

  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(table);

  return container;
}

QWidget *TradingBottomPanel::createOpenOrdersTab() {
  QStringList headers = {"Time",      "Symbol", "Type",   "Side",
                         "Price",     "Amount", "Filled", "Reduce Only",
                         "Post Only", "Status"};
  QTableWidget *table = createTable(headers);

  QWidget *container = new QWidget();
  container->setStyleSheet("background-color: #161616;");
  container->setAutoFillBackground(true);
  QPalette pal = container->palette();
  pal.setColor(QPalette::Window, QColor("#161616"));
  container->setPalette(pal);

  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(table);

  return container;
}

QWidget *TradingBottomPanel::createOrderHistoryTab() {
  QStringList headers = {
      "Time",          "Symbol",          "Type",
      "Average/Price", "Executed/Amount", "Trigger conditions",
      "Reduce Only",   "Post Only",       "Status"};
  QTableWidget *table = createTable(headers);

  // Hardcoded data
  int row = table->rowCount();
  table->insertRow(row);
  table->setItem(row, 0, new QTableWidgetItem("2025-11-25\n10:13:00"));

  QTableWidgetItem *sym1 = new QTableWidgetItem("BTCUSDT\nBuy");
  sym1->setForeground(QBrush(QColor("#3498db")));
  table->setItem(row, 1, sym1);

  table->setItem(row, 2, new QTableWidgetItem("Limit"));
  table->setItem(row, 3, new QTableWidgetItem("-\n85,975.7"));
  table->setItem(row, 4, new QTableWidgetItem("0.000 BTC\n0.001 BTC"));
  table->setItem(row, 5, new QTableWidgetItem("-"));
  table->setItem(row, 6, new QTableWidgetItem("No"));
  table->setItem(row, 7, new QTableWidgetItem("No"));
  table->setItem(row, 8, new QTableWidgetItem("Canceled"));

  row = table->rowCount();
  table->insertRow(row);
  table->setItem(row, 0, new QTableWidgetItem("2025-11-25\n10:12:04"));

  QTableWidgetItem *sym2 = new QTableWidgetItem("BTCUSDT\nBuy");
  sym2->setForeground(QBrush(QColor("#3498db")));
  table->setItem(row, 1, sym2);

  table->setItem(row, 2, new QTableWidgetItem("Limit"));
  table->setItem(row, 3, new QTableWidgetItem("-\n85,958.2"));
  table->setItem(row, 4, new QTableWidgetItem("0.000 BTC\n0.001 BTC"));
  table->setItem(row, 5, new QTableWidgetItem("-"));
  table->setItem(row, 6, new QTableWidgetItem("No"));
  table->setItem(row, 7, new QTableWidgetItem("No"));
  table->setItem(row, 8, new QTableWidgetItem("Canceled"));

  QWidget *container = new QWidget();
  container->setStyleSheet("background-color: #161616;");
  container->setAutoFillBackground(true);
  QPalette pal = container->palette();
  pal.setColor(QPalette::Window, QColor("#161616"));
  container->setPalette(pal);

  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(table);

  return container;
}

QWidget *TradingBottomPanel::createTradeHistoryTab() {
  QStringList headers = {"Time",   "Symbol", "Side", "Price",
                         "Amount", "Fee",    "Role"};
  QTableWidget *table = createTable(headers);

  QWidget *container = new QWidget();
  container->setStyleSheet("background-color: #161616;");
  container->setAutoFillBackground(true);
  QPalette pal = container->palette();
  pal.setColor(QPalette::Window, QColor("#161616"));
  container->setPalette(pal);

  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(table);

  return container;
}

QWidget *TradingBottomPanel::createTransactionHistoryTab() {
  QStringList headers = {"Time", "Type", "Amount", "Asset", "Status", "TxID"};
  QTableWidget *table = createTable(headers);

  QWidget *container = new QWidget();
  container->setStyleSheet("background-color: #161616;");
  container->setAutoFillBackground(true);
  QPalette pal = container->palette();
  pal.setColor(QPalette::Window, QColor("#161616"));
  container->setPalette(pal);

  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(table);

  return container;
}

QWidget *TradingBottomPanel::createDepositsWithdrawalsTab() {
  QStringList headers = {"Asset",   "Network", "Amount", "Type",
                         "Account", "Date",    "txid",   "Status"};
  QTableWidget *table = createTable(headers);

  int row = table->rowCount();
  table->insertRow(row);

  QTableWidgetItem *asset = new QTableWidgetItem("USDC");
  asset->setIcon(QIcon(":/icons/usdc.png"));
  table->setItem(row, 0, asset);

  table->setItem(row, 1, new QTableWidgetItem("BNB Chain"));
  table->setItem(row, 2, new QTableWidgetItem("10.00000000"));
  table->setItem(row, 3, new QTableWidgetItem("DEPOSIT"));
  table->setItem(row, 4, new QTableWidgetItem("Spot"));
  table->setItem(row, 5, new QTableWidgetItem("2025-11-07 15:40:35"));

  QTableWidgetItem *txid = new QTableWidgetItem("0xb...0d2");
  txid->setForeground(QBrush(QColor("#e67e22")));
  table->setItem(row, 6, txid);

  table->setItem(row, 7, new QTableWidgetItem("Success"));

  QWidget *container = new QWidget();
  container->setStyleSheet("background-color: #161616;");
  container->setAutoFillBackground(true);
  QPalette pal = container->palette();
  pal.setColor(QPalette::Window, QColor("#161616"));
  container->setPalette(pal);

  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(table);

  return container;
}

QWidget *TradingBottomPanel::createAssetsTab() {
  QStringList headers = {"Asset", "Wallet Balance", "Unrealized PNL",
                         "Margin Balance", "Asset Contribution"};
  QTableWidget *table = createTable(headers);

  int row = table->rowCount();
  table->insertRow(row);
  table->setItem(row, 0, new QTableWidgetItem("USDC"));
  table->setItem(row, 1, new QTableWidgetItem("10.00000000"));
  table->setItem(row, 2, new QTableWidgetItem("0.00000000"));
  table->setItem(row, 3, new QTableWidgetItem("10.00000000"));
  table->setItem(row, 4, new QTableWidgetItem("9.99729970 USD"));


  QWidget *container = new QWidget();
  container->setStyleSheet("background-color: #161616;");
  container->setAutoFillBackground(true);
  QPalette pal = container->palette();
  pal.setColor(QPalette::Window, QColor("#161616"));
  container->setPalette(pal);

  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(table);

  return container;
}
