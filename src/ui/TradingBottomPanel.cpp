#include "TradingBottomPanel.h"
#include <QBrush>
#include <QColor>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QPalette>
#include <QVBoxLayout>
#include <QDateTime>
#include <QPushButton>


TradingBottomPanel::TradingBottomPanel(QWidget *parent) : QTabWidget(parent) {
  setupTabs();
  setupStyle();
}

void TradingBottomPanel::setupTabs() {
  addTab(createPositionsTab(), "Positions (0)");
  addTab(createOpenOrdersTab(), "Open orders (0)");
  addTab(createOrderHistoryTab(), "Order history");
  addTab(createTradeHistoryTab(), "Trade history");
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
      "Symbol",     "Size (Coin)", "Size (USDC)",      "Entry price",        "Mark price",
      "PNL (ROE%)", "TP/SL for position", "Action"};
  m_positionsTable = createTable(headers);
  
  // Make Action column slightly wider for the close button
  m_positionsTable->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Fixed);
  m_positionsTable->setColumnWidth(7, 80);

  QWidget *container = new QWidget();
  container->setStyleSheet("background-color: #161616;");
  container->setAutoFillBackground(true);
  QPalette pal = container->palette();
  pal.setColor(QPalette::Window, QColor("#161616"));
  container->setPalette(pal);

  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_positionsTable);

  return container;
}

void TradingBottomPanel::addPosition(const QString &symbol, const QString &type, const QString &side, const QString &price, const QString &amount) {
    if (!m_positionsTable) return;
    
    // For this mock, we only add Market orders to Positions directly
    // (In reality, Limit orders go to Open Orders and execute later)
    if (type != "Market") return;
    
    int row = 0; 
    m_positionsTable->insertRow(row);
    
    QTableWidgetItem *symItem = new QTableWidgetItem(symbol);
    symItem->setForeground(QBrush(side == "Buy" ? QColor("#2db9b9") : QColor("#e24a6d")));
    symItem->setData(Qt::UserRole, side);
    m_positionsTable->setItem(row, 0, symItem);
    
    double entryPriceDbl = price.toDouble();
    if (entryPriceDbl <= 0.0) entryPriceDbl = 96000.0; // Fallback if parsing fails, though not expected anymore
    
    double sizeToken = amount.split(" ").first().toDouble();
    QString tokenSymbol = amount.split(" ").last();
    double sizeUsdc = sizeToken * entryPriceDbl;
    
    QTableWidgetItem *sizeCoinItem = new QTableWidgetItem(QString("%1 %2").arg(QString::number(sizeToken, 'f', 5), tokenSymbol));
    sizeCoinItem->setData(Qt::UserRole, sizeToken);
    m_positionsTable->setItem(row, 1, sizeCoinItem);
    
    QTableWidgetItem *sizeUsdcItem = new QTableWidgetItem(QString("%1 USDC").arg(QString::number(sizeUsdc, 'f', 2)));
    sizeUsdcItem->setData(Qt::UserRole, sizeUsdc);
    m_positionsTable->setItem(row, 2, sizeUsdcItem);
    
    QTableWidgetItem *entryItem = new QTableWidgetItem(QString::number(entryPriceDbl, 'f', 2));
    entryItem->setData(Qt::UserRole, entryPriceDbl);
    m_positionsTable->setItem(row, 3, entryItem);
    
    m_positionsTable->setItem(row, 4, new QTableWidgetItem(QString::number(entryPriceDbl, 'f', 2)));
    
    QTableWidgetItem *pnlItem = new QTableWidgetItem("+0.00 (+0.00%)");
    pnlItem->setForeground(QBrush(QColor("#2db9b9")));
    m_positionsTable->setItem(row, 5, pnlItem);
    
    m_positionsTable->setItem(row, 6, new QTableWidgetItem("-- / --"));
    
    QPushButton *closeBtn = new QPushButton("Close");
    closeBtn->setStyleSheet(
        "QPushButton { background-color: #1e1e1e; color: #fff; border: 1px solid #333; border-radius: 4px; padding: 4px 8px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2a2a2a; border: 1px solid #555; }"
        "QPushButton:pressed { background-color: #111111; border: 1px solid #222; }"
    );
    closeBtn->setCursor(Qt::PointingHandCursor);
    m_positionsTable->setCellWidget(row, 7, closeBtn);
    
    // Connect close button to remove the row and refund balance
    connect(closeBtn, &QPushButton::clicked, this, [this, closeBtn]() {
        for (int i = 0; i < m_positionsTable->rowCount(); ++i) {
            if (m_positionsTable->cellWidget(i, 7) == closeBtn) {
                // Get size and entry price to calculate base cost
                QTableWidgetItem *symItem = m_positionsTable->item(i, 0);
                QTableWidgetItem *sizeItem = m_positionsTable->item(i, 1);
                QTableWidgetItem *entryItem = m_positionsTable->item(i, 3);
                QTableWidgetItem *markItem = m_positionsTable->item(i, 4);
                
                if (symItem && sizeItem && entryItem && markItem) {
                    bool isBuy = (symItem->data(Qt::UserRole).toString() == "Buy");
                    double size = sizeItem->data(Qt::UserRole).toDouble(); // Token amount
                    double entryPrice = entryItem->data(Qt::UserRole).toDouble();
                    double currentPrice = markItem->text().toDouble();
                    
                    double baseCostUsdc = size * entryPrice;
                    double pnl = isBuy ? (currentPrice - entryPrice) * size : (entryPrice - currentPrice) * size;
                    
                    
                    double refundAmount = baseCostUsdc + pnl;
                    emit positionClosed(refundAmount);
                    
                    // Add to Trade History
                    if (m_tradeHistoryTable) {
                        int tr = 0; // Insert at top
                        m_tradeHistoryTable->insertRow(tr);
                        
                        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd\nHH:mm:ss");
                        m_tradeHistoryTable->setItem(tr, 0, new QTableWidgetItem(currentTime));
                        
                        QTableWidgetItem *histSym = new QTableWidgetItem(symItem->text());
                        histSym->setForeground(symItem->foreground());
                        m_tradeHistoryTable->setItem(tr, 1, histSym);
                        
                        QTableWidgetItem *histSide = new QTableWidgetItem(isBuy ? "Buy" : "Sell");
                        histSide->setForeground(symItem->foreground());
                        m_tradeHistoryTable->setItem(tr, 2, histSide);
                        
                        m_tradeHistoryTable->setItem(tr, 3, new QTableWidgetItem(QString::number(entryPrice, 'f', 2)));
                        m_tradeHistoryTable->setItem(tr, 4, new QTableWidgetItem(QString::number(currentPrice, 'f', 2)));
                        m_tradeHistoryTable->setItem(tr, 5, new QTableWidgetItem(sizeItem->text()));
                        
                        QTableWidgetItem *histPnl = new QTableWidgetItem(QString("%1%2 USDC").arg(pnl >= 0 ? "+" : "").arg(QString::number(pnl, 'f', 2)));
                        histPnl->setForeground(QBrush(pnl >= 0 ? QColor("#2db9b9") : QColor("#e24a6d")));
                        m_tradeHistoryTable->setItem(tr, 6, histPnl);
                    }
                }

                m_positionsTable->removeRow(i);
                setTabText(0, QString("Positions (%1)").arg(m_positionsTable->rowCount()));
                break;
            }
        }
    });
    
    setTabText(0, QString("Positions (%1)").arg(m_positionsTable->rowCount()));
}

void TradingBottomPanel::updateMarkPrices(double currentPrice) {
    if (!m_positionsTable) return;
    
    double totalUnrealizedPnl = 0.0;
    
    for (int i = 0; i < m_positionsTable->rowCount(); ++i) {
        // Retrieve stored data
        QTableWidgetItem *symItem = m_positionsTable->item(i, 0);
        QTableWidgetItem *sizeItem = m_positionsTable->item(i, 1);
        QTableWidgetItem *entryItem = m_positionsTable->item(i, 3);
        QTableWidgetItem *markItem = m_positionsTable->item(i, 4);
        QTableWidgetItem *pnlItem = m_positionsTable->item(i, 5);
        
        if (!symItem || !sizeItem || !entryItem || !markItem || !pnlItem) continue;
        
        bool isBuy = (symItem->data(Qt::UserRole).toString() == "Buy");
        double size = sizeItem->data(Qt::UserRole).toDouble(); // Token amount
        double entryPrice = entryItem->data(Qt::UserRole).toDouble();
        
        // Update Mark Price text
        markItem->setText(QString::number(currentPrice, 'f', 2));
        
        // Calculate PNL
        double pnl = isBuy ? (currentPrice - entryPrice) * size : (entryPrice - currentPrice) * size;
        totalUnrealizedPnl += pnl;
        
        double roe = (entryPrice > 0 && size > 0) ? (pnl / (entryPrice * size)) * 100.0 : 0.0;
        
        QString pnlStr = QString("%1%2 (%3%4%)")
                            .arg(pnl >= 0 ? "+" : "")
                            .arg(QString::number(pnl, 'f', 2))
                            .arg(roe >= 0 ? "+" : "")
                            .arg(QString::number(roe, 'f', 2));
        
        pnlItem->setText(pnlStr);
        pnlItem->setForeground(QBrush(pnl >= 0 ? QColor("#2db9b9") : QColor("#e24a6d")));
    }
    
    emit unrealizedPnlUpdated(totalUnrealizedPnl);
}

QWidget *TradingBottomPanel::createOpenOrdersTab() {
  QStringList headers = {"Time",      "Symbol", "Type",   "Side",
                         "Price",     "Amount", "Filled", "Reduce Only",
                         "Post Only", "Status"};
  m_openOrdersTable = createTable(headers);

  QWidget *container = new QWidget();
  container->setStyleSheet("background-color: #161616;");
  container->setAutoFillBackground(true);
  QPalette pal = container->palette();
  pal.setColor(QPalette::Window, QColor("#161616"));
  container->setPalette(pal);

  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_openOrdersTable);

  return container;
}

void TradingBottomPanel::addOpenOrder(const QString &symbol, const QString &type, const QString &side, const QString &price, const QString &amount) {
    if (!m_openOrdersTable) return;
    
    int row = 0; 
    m_openOrdersTable->insertRow(row);
    
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd\nHH:mm:ss");
    m_openOrdersTable->setItem(row, 0, new QTableWidgetItem(timeStr));
    
    QTableWidgetItem *symItem = new QTableWidgetItem(symbol);
    symItem->setForeground(QBrush(side == "Buy" ? QColor("#2db9b9") : QColor("#e24a6d")));
    m_openOrdersTable->setItem(row, 1, symItem);
    
    m_openOrdersTable->setItem(row, 2, new QTableWidgetItem(type));
    
    QTableWidgetItem *sideItem = new QTableWidgetItem(side);
    sideItem->setForeground(QBrush(side == "Buy" ? QColor("#2db9b9") : QColor("#e24a6d")));
    m_openOrdersTable->setItem(row, 3, sideItem);
    
    m_openOrdersTable->setItem(row, 4, new QTableWidgetItem(price));
    m_openOrdersTable->setItem(row, 5, new QTableWidgetItem(amount));
    m_openOrdersTable->setItem(row, 6, new QTableWidgetItem("0.00%"));
    m_openOrdersTable->setItem(row, 7, new QTableWidgetItem("No"));
    m_openOrdersTable->setItem(row, 8, new QTableWidgetItem("No"));
    m_openOrdersTable->setItem(row, 9, new QTableWidgetItem("New"));
    
    setTabText(1, QString("Open orders (%1)").arg(m_openOrdersTable->rowCount()));
}

QWidget *TradingBottomPanel::createOrderHistoryTab() {
  QStringList headers = {
      "Time", "Symbol", "Type",
      "Trigger conditions", "Status"};
  QTableWidget *table = createTable(headers);

  // Hardcoded data
  int row = table->rowCount();
  table->insertRow(row);
  table->setItem(row, 0, new QTableWidgetItem("2025-11-25\n10:13:00"));

  QTableWidgetItem *sym1 = new QTableWidgetItem("BTCUSDT\nBuy");
  sym1->setForeground(QBrush(QColor("#3498db")));
  table->setItem(row, 1, sym1);

  table->setItem(row, 2, new QTableWidgetItem("Limit"));
  table->setItem(row, 3, new QTableWidgetItem("-"));
  table->setItem(row, 4, new QTableWidgetItem("Canceled"));

  row = table->rowCount();
  table->insertRow(row);
  table->setItem(row, 0, new QTableWidgetItem("2025-11-25\n10:12:04"));

  QTableWidgetItem *sym2 = new QTableWidgetItem("BTCUSDT\nBuy");
  sym2->setForeground(QBrush(QColor("#3498db")));
  table->setItem(row, 1, sym2);

  table->setItem(row, 2, new QTableWidgetItem("Limit"));
  table->setItem(row, 3, new QTableWidgetItem("-"));
  table->setItem(row, 4, new QTableWidgetItem("Canceled"));

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
  QStringList headers = {"Time", "Symbol", "Side", "Entry Price", "Close Price", "Amount", "PNL"};
  m_tradeHistoryTable = createTable(headers);

  QWidget *container = new QWidget();
  container->setStyleSheet("background-color: #161616;");
  container->setAutoFillBackground(true);
  QPalette pal = container->palette();
  pal.setColor(QPalette::Window, QColor("#161616"));
  container->setPalette(pal);

  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_tradeHistoryTable);

  return container;
}

QWidget *TradingBottomPanel::createAssetsTab() {
  QStringList headers = {"Asset", "Wallet Balance", "Unrealized PNL"};
  m_assetsTable = createTable(headers);

  int row = m_assetsTable->rowCount();
  m_assetsTable->insertRow(row);
  m_assetsTable->setItem(row, 0, new QTableWidgetItem("USDC"));
  m_assetsTable->setItem(row, 1, new QTableWidgetItem("100.00000000"));
  m_assetsTable->setItem(row, 2, new QTableWidgetItem("0.00000000"));


  QWidget *container = new QWidget();
  container->setStyleSheet("background-color: #161616;");
  container->setAutoFillBackground(true);
  QPalette pal = container->palette();
  pal.setColor(QPalette::Window, QColor("#161616"));
  container->setPalette(pal);

  QVBoxLayout *layout = new QVBoxLayout(container);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_assetsTable);

  return container;
}

void TradingBottomPanel::updateWalletBalance(double balance) {
    if (!m_assetsTable || m_assetsTable->rowCount() == 0) return;
    
    QTableWidgetItem *balanceItem = m_assetsTable->item(0, 1);
    if (balanceItem) {
        balanceItem->setText(QString::number(balance, 'f', 8));
    }
}
