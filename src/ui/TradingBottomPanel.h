#ifndef TRADINGBOTTOMPANEL_H
#define TRADINGBOTTOMPANEL_H

#include <QTabWidget>
#include <QTableWidget>

class TradingBottomPanel : public QTabWidget {
  Q_OBJECT

public:
  explicit TradingBottomPanel(QWidget *parent = nullptr);

private:
  void setupTabs();
  void setupStyle();

  // Helper to create specific tables
  QWidget *createPositionsTab();
  QWidget *createOpenOrdersTab();
  QWidget *createOrderHistoryTab();
  QWidget *createTradeHistoryTab();
  QWidget *createTransactionHistoryTab();
  QWidget *createDepositsWithdrawalsTab();
  QWidget *createAssetsTab();

  // Helper to setup a standard table
  QTableWidget *createTable(const QStringList &headers);
};

#endif // TRADINGBOTTOMPANEL_H
