#include "orderbook.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFile>
#include <QDebug>
#include <fstream>
#include <iostream>
#include <algorithm>

using json = nlohmann::json;

OrderBook::OrderBook(QWidget *parent) : QWidget(parent), currentFileIndex(0) {
    setupUi();
    
    // Initialize simulation timer
    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &OrderBook::updateOrderBook);
    simulationTimer->start(1000); // Update every 1 second
    
    // Initial load
    updateOrderBook();
}

OrderBook::~OrderBook() {
}

void OrderBook::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Header Row (Custom Labels to match screenshot)
    QWidget* headerWidget = new QWidget(this);
    headerWidget->setStyleSheet("background-color: #161616;");
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(0, 5, 0, 5);
    headerLayout->setSpacing(0);
    
    QLabel* lblPrice = new QLabel("Price", this);
    QLabel* lblSize = new QLabel("Amount", this);
    QLabel* lblTotal = new QLabel("Total", this);
    
    QString headerStyle = "color: #888; font-size: 9pt; font-weight: bold;";
    lblPrice->setStyleSheet(headerStyle);
    lblSize->setStyleSheet(headerStyle);
    lblTotal->setStyleSheet(headerStyle);
    
    lblPrice->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    // Screenshot: "Price" label is left, numbers are right? Let's check screenshot again... I can't.
    // Standard: Numbers right aligned. Labels usually match.
    // Let's use Stretch for spacing.
    
    headerLayout->addWidget(lblPrice, 1, Qt::AlignRight);
    headerLayout->addWidget(lblSize, 1, Qt::AlignRight);
    headerLayout->addWidget(lblTotal, 1, Qt::AlignRight);
    
    mainLayout->addWidget(headerWidget);

    // Spacer to push Asks down (so they align to bottom of this section, near Spread)
    // This stretch combined with one after bids centers the orderbook vertically
    mainLayout->addStretch(1);

    // Asks Table (Top)
    asksTable = new QTableWidget(this);
    asksTable->setColumnCount(3);
    asksTable->horizontalHeader()->setVisible(false);
    asksTable->verticalHeader()->setVisible(false);
    asksTable->setShowGrid(false);
    asksTable->setSelectionMode(QAbstractItemView::NoSelection);
    asksTable->setFocusPolicy(Qt::NoFocus);
    asksTable->setStyleSheet("background-color: #161616; border: none;");
    asksTable->setItemDelegate(new DepthDelegate(asksTable));
    asksTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    asksTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // Fixed height to content
    mainLayout->addWidget(asksTable, 0); // No stretch for table itself

    // Spread Label (Middle)
    spreadLabel = new QLabel(this);
    spreadLabel->setAlignment(Qt::AlignCenter);
    spreadLabel->setStyleSheet("color: #aaa; font-size: 10pt; padding: 5px; background-color: #202020; font-family: Consolas, monospace;");
    mainLayout->addWidget(spreadLabel);

    // Bids Table (Bottom)
    bidsTable = new QTableWidget(this);
    bidsTable->setColumnCount(3);
    bidsTable->horizontalHeader()->setVisible(false);
    bidsTable->verticalHeader()->setVisible(false);
    bidsTable->setShowGrid(false);
    bidsTable->setSelectionMode(QAbstractItemView::NoSelection);
    bidsTable->setFocusPolicy(Qt::NoFocus);
    bidsTable->setStyleSheet("background-color: #161616; border: none;");
    bidsTable->setItemDelegate(new DepthDelegate(bidsTable));
    bidsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    bidsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // Fixed height to content
    mainLayout->addWidget(bidsTable, 0); // No stretch for table itself

    // Spacer after bids to center the orderbook vertically
    mainLayout->addStretch(1);
}

void OrderBook::updateOrderBook() {
    currentFileIndex = (currentFileIndex % 3) + 1;
    std::string filename = "orderbook_" + std::to_string(currentFileIndex) + ".json";
    loadData(filename);
}

void OrderBook::loadData(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        qDebug() << "Failed to open " << QString::fromStdString(filename);
        return;
    }

    try {
        json j;
        file >> j;

        // Calculate Max Total for Depth Bars across both books (or per book? usually per side)
        // Let's calculate max total per side to scale bars.
        double maxTotal = 0; // Not used here, calculated in populate

        // Populate Asks (Top)
        // Asks should be sorted DESCENDING for display (Highest Price at Top, Lowest at Bottom near spread)
        // Wait, standard vertical order book:
        // Asks: High -> Low (Lowest near spread)
        // Spread
        // Bids: High -> Low (Highest near spread)
        
        // So Asks table: Row 0 is Highest Price. Last Row is Lowest Price (Best Ask).
        // Bids table: Row 0 is Highest Price (Best Bid). Last Row is Lowest Price.
        
        if (j.contains("asks")) {
            std::vector<json> asks = j["asks"].get<std::vector<json>>();
            // Sort Asks Ascending (Lowest first) to find the best asks
            std::sort(asks.begin(), asks.end(), [](const json& a, const json& b) {
                return a["price"] < b["price"];
            });
            
            // Keep only top 7 (configurable depth)
            const size_t DEPTH = 7;
            if (asks.size() > DEPTH) {
                asks.resize(DEPTH);
            }
            
            // Sort Asks Descending (Highest first) for display (Vertical Layout: High -> Low)
            std::sort(asks.begin(), asks.end(), [](const json& a, const json& b) {
                return a["price"] > b["price"];
            });
            
            populateTable(asksTable, asks, false, maxTotal);
        }
        
        if (j.contains("bids")) {
            std::vector<json> bids = j["bids"].get<std::vector<json>>();
            // Sort Bids Descending (Highest first) - Best bids are highest
            std::sort(bids.begin(), bids.end(), [](const json& a, const json& b) {
                return a["price"] > b["price"];
            });
            
            // Keep only top 7 (configurable depth)
            const size_t DEPTH = 7;
            if (bids.size() > DEPTH) {
                bids.resize(DEPTH);
            }
            
            populateTable(bidsTable, bids, true, maxTotal);
        }

        // Calculate Spread
        if (bidsTable->rowCount() > 0 && asksTable->rowCount() > 0) {
            // Best Ask is the LAST row of Asks table
            double bestAsk = asksTable->item(asksTable->rowCount() - 1, 0)->text().replace(" ", "").toDouble();
            // Best Bid is the FIRST row of Bids table
            double bestBid = bidsTable->item(0, 0)->text().replace(" ", "").toDouble();
            
            double spread = bestAsk - bestBid;
            double percentage = (spread / bestAsk) * 100.0;
            double fairPrice = (bestAsk + bestBid) / 2.0;
            spreadLabel->setText(QString("%1  %2%       Fair %3")
                                 .arg(formatNumber(spread, 1))
                                 .arg(formatNumber(percentage, 3))
                                 .arg(formatNumber(fairPrice, 1)));
        }

    } catch (const std::exception& e) {
        qDebug() << "JSON Parse Error: " << e.what();
    }
}

void OrderBook::populateTable(QTableWidget* table, const std::vector<nlohmann::json>& data, bool isBid, double& maxTotal) {
    table->setRowCount(0); // Clear existing
    
    // Calculate Cumulative Totals first to find Max
    std::vector<double> totals;
    double currentTotal = 0;
    
    // For Asks (displayed High to Low), the cumulative sum usually starts from the Best Ask (Lowest Price).
    // So if we iterate High to Low (0 to N), we are going away from spread?
    // No, "Total" usually means "Sum of size from Best Price up to this level".
    // Best Ask is Lowest Price. Best Bid is Highest Price.
    
    // If table is Asks (High -> Low):
    // We need to sum from Bottom (Lowest Price) up to Top (Highest Price).
    // So iterate backwards for Asks?
    
    // If table is Bids (High -> Low):
    // Best Bid is Top (Highest Price).
    // Sum starts from Top.
    
    totals.resize(data.size());
    if (isBid) {
        // Bids: Sorted High to Low. Top is Best.
        // Sum from 0 to N.
        for (size_t i = 0; i < data.size(); ++i) {
            currentTotal += (double)data[i]["quantity"];
            totals[i] = currentTotal;
        }
    } else {
        // Asks: Sorted High to Low. Bottom is Best.
        // Sum from N-1 down to 0.
        for (int i = data.size() - 1; i >= 0; --i) {
            currentTotal += (double)data[i]["quantity"];
            totals[i] = currentTotal;
        }
    }
    
    maxTotal = currentTotal; // Max cumulative total

    int row = 0;
    for (size_t i = 0; i < data.size(); ++i) {
        table->insertRow(row);
        
        double price = data[i]["price"];
        double qty = data[i]["quantity"];
        double total = totals[i];
        
        QTableWidgetItem* priceItem = new QTableWidgetItem(formatNumber(price, 0)); // BTC Price usually no decimals if > 1000? Or 1. Screenshot shows 92400.
        QTableWidgetItem* qtyItem = new QTableWidgetItem(formatBTC(qty));
        QTableWidgetItem* totalItem = new QTableWidgetItem(formatBTC(total));
        
        // Set UserData for Delegate
        // We want depth bars on the Total column? Or all columns? Screenshot shows bars behind the text across the row?
        // "two-column table-like view... showing price and quantity per level"
        // Screenshot shows bars behind "Total" and "Size"?
        // Let's apply data to all items so delegate can draw background on all.
        
        for (auto* item : {priceItem, qtyItem, totalItem}) {
            item->setData(Qt::UserRole, total); // Value for bar
            item->setData(Qt::UserRole + 1, maxTotal); // Max for scaling
            item->setData(Qt::UserRole + 2, isBid); // Side
        }

        table->setItem(row, 0, priceItem);
        table->setItem(row, 1, qtyItem);
        table->setItem(row, 2, totalItem);
        
        row++;
    }

    // Resize table height to fit content (if it's the Asks table or we want tight layout)
    // Especially important for Asks table to let the spacer above it work.
    if (table->rowCount() > 0) {
        int height = 0;
        for (int i=0; i<table->rowCount(); ++i) height += table->rowHeight(i);
        // Add a bit for margins if needed, or rely on precise calc
        table->setFixedHeight(height);
    } else {
        table->setFixedHeight(0);
    }
}

QString OrderBook::formatNumber(double value, int decimals) {
    // Add thousands separator?
    QString str = QString::number(value, 'f', decimals);
    // Simple space separator for thousands
    int pos = str.indexOf('.');
    if (pos == -1) pos = str.length();
    while (pos > 3) {
        pos -= 3;
        str.insert(pos, ' ');
    }
    return str;
}

QString OrderBook::formatBTC(double value) {
    return QString::number(value, 'f', 4); // 4 decimals for BTC size
}
