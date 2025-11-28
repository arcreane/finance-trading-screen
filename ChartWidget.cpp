#include "ChartWidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QVBoxLayout>


ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent) {
  setupChart();

  // Auto-load BTC data as requested
  loadData("BTC", "Daily");
}

ChartWidget::~ChartWidget() {}

void ChartWidget::setupChart() {
  chart = new QChart();
  chart->setTitle("BTC Daily Chart");
  chart->setTheme(QChart::ChartThemeDark);
  chart->legend()->setVisible(true);
  chart->legend()->setAlignment(Qt::AlignBottom);

  chartView = new QChartView(chart);
  chartView->setRenderHint(QPainter::Antialiasing);

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(chartView);

  series = new QCandlestickSeries();
  series->setName("BTC");
  series->setIncreasingColor(QColor(Qt::green));
  series->setDecreasingColor(QColor(Qt::red));
  series->setBodyOutlineVisible(false);

  chart->addSeries(series);

  axisX = new QDateTimeAxis();
  axisX->setFormat("dd-MM-yyyy");
  axisX->setTitleText("Date");
  chart->addAxis(axisX, Qt::AlignBottom);
  series->attachAxis(axisX);

  axisY = new QValueAxis();
  axisY->setTitleText("Price");
  chart->addAxis(axisY, Qt::AlignLeft);
  series->attachAxis(axisY);
}

bool ChartWidget::connectToDatabase() {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName("backtest.db");

  if (!db.open()) {
    qDebug() << "Error: connection with database failed";
    return false;
  } else {
    qDebug() << "Database: connection ok";
    return true;
  }
}

void ChartWidget::loadData(const QString &symbol, const QString &interval) {
  if (!connectToDatabase()) {
    return;
  }

  QSqlQuery query;
  // Assuming 'Daily' means we take all data points for now, or we could filter
  // if there were multiple intervals The table has 'Timestamp', 'Open', 'High',
  // 'Low', 'Close'
  query.prepare("SELECT Timestamp, Open, High, Low, Close FROM StockData WHERE "
                "Symbol = :symbol ORDER BY Timestamp");
  query.bindValue(":symbol", symbol);

  if (!query.exec()) {
    qDebug() << "Query failed:" << query.lastError();
    return;
  }

  series->clear();

  qint64 minTimestamp = std::numeric_limits<qint64>::max();
  qint64 maxTimestamp = std::numeric_limits<qint64>::min();
  double minPrice = std::numeric_limits<double>::max();
  double maxPrice = std::numeric_limits<double>::min();

  while (query.next()) {
    QString timestampStr = query.value(0).toString();
    double open = query.value(1).toDouble();
    double high = query.value(2).toDouble();
    double low = query.value(3).toDouble();
    double close = query.value(4).toDouble();

    // Parse timestamp. Format in DB is 'YYYY-MM-DD HH:MM:SS'
    QDateTime timestamp =
        QDateTime::fromString(timestampStr, "yyyy-MM-dd HH:mm:ss");
    if (!timestamp.isValid()) {
      // Try alternate format just in case
      timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
    }

    if (timestamp.isValid()) {
      QCandlestickSet *set = new QCandlestickSet(timestamp.toMSecsSinceEpoch());
      set->setOpen(open);
      set->setHigh(high);
      set->setLow(low);
      set->setClose(close);

      series->append(set);

      if (timestamp.toMSecsSinceEpoch() < minTimestamp)
        minTimestamp = timestamp.toMSecsSinceEpoch();
      if (timestamp.toMSecsSinceEpoch() > maxTimestamp)
        maxTimestamp = timestamp.toMSecsSinceEpoch();
      if (low < minPrice)
        minPrice = low;
      if (high > maxPrice)
        maxPrice = high;
    }
  }

  if (series->count() > 0) {
    axisX->setRange(QDateTime::fromMSecsSinceEpoch(minTimestamp),
                    QDateTime::fromMSecsSinceEpoch(maxTimestamp));
    axisY->setRange(minPrice * 0.95, maxPrice * 1.05); // Add some padding
  }
}
