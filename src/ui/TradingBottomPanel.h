#ifndef TRADINGBOTTOMPANEL_H
#define TRADINGBOTTOMPANEL_H

#include <QTabWidget>
#include <QTableWidget>

class TradingBottomPanel : public QTabWidget {
  Q_OBJECT

public:
  explicit TradingBottomPanel(QWidget *parent = nullptr);

signals:
  void positionClosed(double refundAmount);
  void unrealizedPnlUpdated(double pnl);

public slots:
  void addOpenOrder(const QString &symbol, const QString &type, const QString &side, const QString &price, const QString &amount);
  void addPosition(const QString &symbol, const QString &type, const QString &side, const QString &price, const QString &amount);
  void updateMarkPrices(double currentPrice);
  void updateWalletBalance(double balance);

private:
  QTableWidget *m_positionsTable = nullptr;
  QTableWidget *m_openOrdersTable = nullptr;
  QTableWidget *m_assetsTable = nullptr;
  QTableWidget *m_tradeHistoryTable = nullptr;

  void setupTabs();
  void setupStyle();

  // Helper to create specific tables
  QWidget *createPositionsTab();
  QWidget *createOpenOrdersTab();
  QWidget *createOrderHistoryTab();
  QWidget *createTradeHistoryTab();
  QWidget *createAssetsTab();

  // Helper to setup a standard table
  QTableWidget *createTable(const QStringList &headers);
};

#endif // TRADINGBOTTOMPANEL_H
