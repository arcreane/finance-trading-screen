#include "orderbook.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDebug>
#include <QNetworkRequest>
#include <QUrl>
#include <QTableWidgetItem>
#include <algorithm>
#include <cmath>
#include <map>

OrderBook::OrderBook(QWidget *parent)
    : QWidget(parent)
    , m_currentSymbol("BTC")
{
    setupUi();

    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &OrderBook::onHttpResponse);

    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(1000);
    connect(m_pollTimer, &QTimer::timeout, this, &OrderBook::fetchOrderBook);
    m_pollTimer->start();

    fetchOrderBook();
}

OrderBook::~OrderBook() {
    m_pollTimer->stop();
}

void OrderBook::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    asksTable = new QTableWidget(this);
    asksTable->setColumnCount(3);
    asksTable->setHorizontalHeaderLabels({"Price", "Amount", "Total"});

    asksTable->horizontalHeader()->setVisible(true);
    asksTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    asksTable->horizontalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);
    asksTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section { background-color: #161616; color: #848e9c; border: none; font-weight: bold; font-size: 11px; padding: 4px; }"
    );

    asksTable->verticalHeader()->setVisible(false);
    asksTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    asksTable->setShowGrid(false);
    asksTable->setSelectionMode(QAbstractItemView::NoSelection);
    asksTable->setFocusPolicy(Qt::NoFocus);
    asksTable->setStyleSheet("background-color: #161616; border: none;");
    asksTable->setItemDelegate(new DepthDelegate(asksTable));
    asksTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(asksTable, 1);

    spreadLabel = new QLabel(this);
    spreadLabel->setAlignment(Qt::AlignCenter);
    spreadLabel->setStyleSheet("color: #aaa; font-size: 10pt; padding: 5px; background-color: #202020; font-family: Consolas, monospace;");
    mainLayout->addWidget(spreadLabel, 0);

    bidsTable = new QTableWidget(this);
    bidsTable->setColumnCount(3);
    bidsTable->horizontalHeader()->setVisible(false);
    bidsTable->verticalHeader()->setVisible(false);
    bidsTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    bidsTable->setShowGrid(false);
    bidsTable->setSelectionMode(QAbstractItemView::NoSelection);
    bidsTable->setFocusPolicy(Qt::NoFocus);
    bidsTable->setStyleSheet("background-color: #161616; border: none;");
    bidsTable->setItemDelegate(new DepthDelegate(bidsTable));
    bidsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    bidsTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(bidsTable, 1);
}

QString OrderBook::buildRequestUrl() {
    QString symbol = m_currentSymbol.toUpper() + "USDT";
    return QString("https://api.binance.com/api/v3/depth?symbol=%1&limit=100").arg(symbol);
}

void OrderBook::fetchOrderBook() {
    QString url = buildRequestUrl();
    qDebug() << "Fetching orderbook:" << url;
    QNetworkRequest request{QUrl(url)};
    m_networkManager->get(request);
}

void OrderBook::onHttpResponse(QNetworkReply* reply) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "HTTP error:" << reply->errorString();
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode != 200) {
        qDebug() << "HTTP status:" << statusCode;
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonObject obj = doc.object();
    if (!obj.contains("asks") || !obj["asks"].isArray() ||
        !obj.contains("bids") || !obj["bids"].isArray()) {
        return;
    }

    processDepthData(obj);
}

void OrderBook::setSymbol(const QString& symbol) {
    if (m_currentSymbol != symbol) {
        m_currentSymbol = symbol;
        fetchOrderBook();
    }
}

void OrderBook::processDepthData(const QJsonObject& json) {
    double maxTotal = 0;

    // Parse raw ask levels
    if (json.contains("asks") && json["asks"].isArray()) {
        QJsonArray asksArray = json["asks"].toArray();
        std::vector<Level> rawAsks;
        rawAsks.reserve(asksArray.size());

        for (const QJsonValue& val : asksArray) {
            QJsonArray entry = val.toArray();
            if (entry.size() >= 2) {
                bool priceOk = false, qtyOk = false;
                double price = entry[0].toString().toDouble(&priceOk);
                double qty = entry[1].toString().toDouble(&qtyOk);
                if (priceOk && qtyOk && price > 0 && qty >= 0) {
                    rawAsks.push_back({price, qty});
                }
            }
        }

        std::vector<Level> grouped = aggregateLevels(rawAsks, false);

        // Take top ORDERBOOK_DEPTH levels (lowest asks = best)
        if ((int)grouped.size() > ORDERBOOK_DEPTH)
            grouped.resize(ORDERBOOK_DEPTH);

        // Reverse for display: highest price at top, best ask at bottom near spread
        std::reverse(grouped.begin(), grouped.end());

        populateTable(asksTable, grouped, false, maxTotal);
    }

    // Parse raw bid levels
    if (json.contains("bids") && json["bids"].isArray()) {
        QJsonArray bidsArray = json["bids"].toArray();
        std::vector<Level> rawBids;
        rawBids.reserve(bidsArray.size());

        for (const QJsonValue& val : bidsArray) {
            QJsonArray entry = val.toArray();
            if (entry.size() >= 2) {
                bool priceOk = false, qtyOk = false;
                double price = entry[0].toString().toDouble(&priceOk);
                double qty = entry[1].toString().toDouble(&qtyOk);
                if (priceOk && qtyOk && price > 0 && qty >= 0) {
                    rawBids.push_back({price, qty});
                }
            }
        }

        std::vector<Level> grouped = aggregateLevels(rawBids, true);

        // Take top ORDERBOOK_DEPTH levels (highest bids = best)
        if ((int)grouped.size() > ORDERBOOK_DEPTH)
            grouped.resize(ORDERBOOK_DEPTH);

        populateTable(bidsTable, grouped, true, maxTotal);
    }

    // Calculate spread from best ask and best bid
    if (bidsTable->rowCount() > 0 && asksTable->rowCount() > 0) {
        auto* askItem = asksTable->item(asksTable->rowCount() - 1, 0);
        auto* bidItem = bidsTable->item(0, 0);
        if (!askItem || !bidItem) return;

        double bestAsk = askItem->text().replace(" ", "").toDouble();
        double bestBid = bidItem->text().replace(" ", "").toDouble();

        if (bestAsk <= 0) return;

        double spread = bestAsk - bestBid;
        double percentage = (spread / bestAsk) * 100.0;
        double fairPrice = (bestAsk + bestBid) / 2.0;
        spreadLabel->setText(QString("%1  %2%       Fair %3")
                             .arg(formatNumber(spread, 1))
                             .arg(formatNumber(percentage, 3))
                             .arg(formatNumber(fairPrice, 1)));
    }
}

std::vector<OrderBook::Level> OrderBook::aggregateLevels(const std::vector<Level>& raw, bool isBid) {
    if (raw.empty()) return {};

    // Compute price range
    double minPrice = raw.front().price;
    double maxPrice = raw.front().price;
    for (const auto& lvl : raw) {
        if (lvl.price < minPrice) minPrice = lvl.price;
        if (lvl.price > maxPrice) maxPrice = lvl.price;
    }

    double range = maxPrice - minPrice;
    if (range <= 0) {
        // All prices identical — return single level with summed qty
        double totalQty = 0;
        for (const auto& lvl : raw) totalQty += lvl.qty;
        return {{raw.front().price, totalQty}};
    }

    double step = computeNiceStep(range, ORDERBOOK_DEPTH);
    if (step <= 0) step = range / ORDERBOOK_DEPTH;

    // Group levels into buckets
    // Bids: floor to step boundary (round down)
    // Asks: ceil to step boundary (round up)
    std::map<double, double> buckets;

    for (const auto& lvl : raw) {
        double bucket;
        if (isBid) {
            bucket = std::floor(lvl.price / step) * step;
        } else {
            bucket = std::ceil(lvl.price / step) * step;
        }
        buckets[bucket] += lvl.qty;
    }

    // Convert to vector and sort
    std::vector<Level> result;
    result.reserve(buckets.size());
    for (const auto& [price, qty] : buckets) {
        if (qty > 0) {
            result.push_back({price, qty});
        }
    }

    if (isBid) {
        // Bids: highest price first (best bid at top)
        std::sort(result.begin(), result.end(), [](const Level& a, const Level& b) {
            return a.price > b.price;
        });
    } else {
        // Asks: lowest price first (best ask first)
        std::sort(result.begin(), result.end(), [](const Level& a, const Level& b) {
            return a.price < b.price;
        });
    }

    return result;
}

double OrderBook::computeNiceStep(double range, int targetBuckets) {
    if (targetBuckets <= 0) return range;

    double rawStep = range / targetBuckets;

    // Find the magnitude (power of 10)
    double magnitude = std::pow(10.0, std::floor(std::log10(rawStep)));

    double normalized = rawStep / magnitude;

    // Round to nearest "nice" number: 1, 2, 5
    double niceNorm;
    if (normalized <= 1.5)
        niceNorm = 1.0;
    else if (normalized <= 3.5)
        niceNorm = 2.0;
    else if (normalized <= 7.5)
        niceNorm = 5.0;
    else
        niceNorm = 10.0;

    return niceNorm * magnitude;
}

void OrderBook::populateTable(QTableWidget* table, const std::vector<Level>& levels, bool isBid, double& maxTotal) {
    table->setRowCount(0);

    // Calculate cumulative totals
    std::vector<double> totals(levels.size());
    double currentTotal = 0;

    if (isBid) {
        // Bids: sorted High to Low, sum from top
        for (size_t i = 0; i < levels.size(); ++i) {
            currentTotal += levels[i].qty;
            totals[i] = currentTotal;
        }
    } else {
        // Asks: sorted High to Low for display, sum from bottom (best ask)
        for (int i = levels.size() - 1; i >= 0; --i) {
            currentTotal += levels[i].qty;
            totals[i] = currentTotal;
        }
    }

    maxTotal = currentTotal;

    for (size_t i = 0; i < levels.size(); ++i) {
        int row = table->rowCount();
        table->insertRow(row);

        QTableWidgetItem* priceItem = new QTableWidgetItem(formatNumber(levels[i].price, 1));
        QTableWidgetItem* qtyItem = new QTableWidgetItem(formatBTC(levels[i].qty));
        QTableWidgetItem* totalItem = new QTableWidgetItem(formatBTC(totals[i]));

        for (auto* item : {priceItem, qtyItem, totalItem}) {
            item->setData(Qt::UserRole, totals[i]);
            item->setData(Qt::UserRole + 1, maxTotal);
            item->setData(Qt::UserRole + 2, isBid);
        }

        table->setItem(row, 0, priceItem);
        table->setItem(row, 1, qtyItem);
        table->setItem(row, 2, totalItem);
    }

    table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

QString OrderBook::formatNumber(double value, int decimals) {
    QString str = QString::number(value, 'f', decimals);
    int pos = str.indexOf('.');
    if (pos == -1) pos = str.length();
    while (pos > 3) {
        pos -= 3;
        str.insert(pos, ' ');
    }
    return str;
}

QString OrderBook::formatBTC(double value) {
    return QString::number(value, 'f', 4);
}
