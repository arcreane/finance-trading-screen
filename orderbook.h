#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QTimer>
#include <QStyledItemDelegate>
#include <QPainter>
#include <vector>
#include "nlohmann/json.hpp"

// Delegate for drawing depth bars
class DepthDelegate : public QStyledItemDelegate {
public:
    DepthDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        // Draw background first (default)
        // QStyledItemDelegate::paint(painter, option, index); 
        // Actually we want to draw our own background bar

        painter->save();

        // Background color (dark)
        painter->fillRect(option.rect, QColor("#161616"));

        // Get data
        double value = index.data(Qt::UserRole).toDouble(); // Current value (Total)
        double maxTotal = index.data(Qt::UserRole + 1).toDouble(); // Max Total in the book
        bool isBid = index.data(Qt::UserRole + 2).toBool();

        if (maxTotal > 0 && value > 0) {
            double ratio = value / maxTotal;
            int width = static_cast<int>(option.rect.width() * ratio);
            
            QRect barRect = option.rect;
            // A bit of spacing between bars and edge if desired, or flush. Flush is standard.
            barRect.setLeft(barRect.right() - width);
            
            // Softer colors
            QColor barColor = isBid ? QColor(14, 203, 129, 40) : QColor(246, 70, 93, 40); 
            painter->fillRect(barRect, barColor);
        }

        // Draw text with padding
        QString text = index.data(Qt::DisplayRole).toString();
        
        // Specific colors for Price column (Column 0)
        if (index.column() == 0) {
             painter->setPen(isBid ? QColor("#0ecb81") : QColor("#f6465d")); // Binance-like Green/Red
        } else {
             painter->setPen(QColor("#c3c5cb")); // Light gray for size/total
        }

        painter->setFont(option.font); // Ensure font is used
        // Add 8px padding from right
        painter->drawText(option.rect.adjusted(0, 0, -8, 0), Qt::AlignRight | Qt::AlignVCenter, text);

        painter->restore();
    }
};

class OrderBook : public QWidget {
    Q_OBJECT

public:
    explicit OrderBook(QWidget *parent = nullptr);
    ~OrderBook();

public slots:
    void setSymbol(const QString& symbol);

private slots:
    void updateOrderBook();

private:
    void setupUi();
    void loadData(const std::string& filename);
    void populateTable(QTableWidget* table, const std::vector<nlohmann::json>& data, bool isBid, double& maxTotal);

    QTableWidget* bidsTable;
    QTableWidget* asksTable;
    QLabel* spreadLabel;
    QTimer* simulationTimer;
    int currentFileIndex;
    QString m_currentSymbol;
    
    // Helper to format numbers
    QString formatNumber(double value, int decimals);
    QString formatBTC(double value);
};

#endif // ORDERBOOK_H
