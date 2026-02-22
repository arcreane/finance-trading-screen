/**
 * @file orderbook.h
 * @brief Order book widget displaying real-time bid/ask depth from Binance.
 *
 * Displays the order book with:
 * - Asks (sell orders) sorted by price
 * - Bids (buy orders) sorted by price
 * - Visual depth bars showing cumulative volume
 * - Spread calculation and fair price display
 * - Real-time data via Binance REST API (polled every 1s)
 * - Price level grouping for readable depth display
 */

#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>

/**
 * @class DepthDelegate
 * @brief Custom delegate for rendering depth bars behind order book rows.
 */
class DepthDelegate : public QStyledItemDelegate {
public:
    DepthDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        painter->save();

        painter->fillRect(option.rect, QColor("#161616"));

        double value = index.data(Qt::UserRole).toDouble();
        double maxTotal = index.data(Qt::UserRole + 1).toDouble();
        bool isBid = index.data(Qt::UserRole + 2).toBool();

        if (maxTotal > 0 && value > 0) {
            double ratio = value / maxTotal;
            int width = static_cast<int>(option.rect.width() * ratio);

            QRect barRect = option.rect;
            barRect.setLeft(barRect.right() - width);

            QColor barColor = isBid ? QColor(14, 203, 129, 40) : QColor(246, 70, 93, 40);
            painter->fillRect(barRect, barColor);
        }

        QString text = index.data(Qt::DisplayRole).toString();

        if (index.column() == 0) {
             painter->setPen(isBid ? QColor("#0ecb81") : QColor("#f6465d"));
        } else {
             painter->setPen(QColor("#c3c5cb"));
        }

        painter->setFont(option.font);
        painter->drawText(option.rect.adjusted(0, 0, -8, 0), Qt::AlignRight | Qt::AlignVCenter, text);

        painter->restore();
    }
};

class OrderBook : public QWidget {
    Q_OBJECT

public:
    static constexpr int ORDERBOOK_DEPTH = 7;

    struct Level {
        double price;
        double qty;
    };

    explicit OrderBook(QWidget *parent = nullptr);
    ~OrderBook();

public slots:
    void setSymbol(const QString& symbol);

private slots:
    void fetchOrderBook();
    void onHttpResponse(QNetworkReply* reply);

private:
    void setupUi();
    void processDepthData(const QJsonObject& json);
    void populateTable(QTableWidget* table, const std::vector<Level>& levels, bool isBid, double& maxTotal);

    std::vector<Level> aggregateLevels(const std::vector<Level>& raw, bool isBid);
    double computeNiceStep(double range, int targetBuckets);
    QString buildRequestUrl();

    QTableWidget* bidsTable;
    QTableWidget* asksTable;
    QLabel* spreadLabel;

    QNetworkAccessManager* m_networkManager;
    QTimer* m_pollTimer;

    QString m_currentSymbol;

    QString formatNumber(double value, int decimals);
    QString formatBTC(double value);
};

#endif // ORDERBOOK_H
