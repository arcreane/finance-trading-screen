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
            // Align bar to the right
            barRect.setLeft(barRect.right() - width);
            
            QColor barColor = isBid ? QColor(0, 255, 0, 30) : QColor(255, 0, 0, 30); // Transparent Green/Red
            painter->fillRect(barRect, barColor);
        }

        // Draw text
        QString text = index.data(Qt::DisplayRole).toString();
        painter->setPen(QColor("#dddddd"));
        
        // Specific colors for Price column (Column 0)
        if (index.column() == 0) {
             painter->setPen(isBid ? QColor("#00ff00") : QColor("#ff4444"));
        }

        painter->drawText(option.rect.adjusted(0, 0, -5, 0), Qt::AlignRight | Qt::AlignVCenter, text);

        painter->restore();
    }
};

class OrderBook : public QWidget {
    Q_OBJECT

public:
    explicit OrderBook(QWidget *parent = nullptr);
    ~OrderBook();

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
    
    // Helper to format numbers
    QString formatNumber(double value, int decimals);
    QString formatBTC(double value);
};

#endif // ORDERBOOK_H
