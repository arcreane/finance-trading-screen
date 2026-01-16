#include "ChartWidget.h"
#include <QtCharts/QAbstractAxis>
#include <QDebug>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QGraphicsLayout>
#include <cmath>

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent) {
  // Main layout
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  setupChart();
  setupRsiChart();

  // Add widgets to layout
  layout->addWidget(chartView, 3); // Main chart takes 75%
  layout->addWidget(rsiChartView, 1); // RSI takes 25%

  // Enable mouse tracking for crosshair
  setMouseTracking(true);
  chartView->setMouseTracking(true);
  rsiChartView->setMouseTracking(true);

  // Auto-load BTC data as requested
  loadData("BTC", "Daily");
}

ChartWidget::~ChartWidget() {}

void ChartWidget::setupChart() {
  chart = new QChart();
  chart->setTitle("");
  
  // Background Colors
  chart->setBackgroundBrush(Qt::NoBrush); // Transparent to show View background
  chart->setPlotAreaBackgroundBrush(Qt::NoBrush);
  chart->setPlotAreaBackgroundVisible(false);
  
  // Title Color
  chart->setTitleBrush(QBrush(QColor("#d1d4dc")));

  chart->legend()->setVisible(false);
  
  // Reduce margins to zero
  chart->setMargins(QMargins(0, 0, 0, 0));
  chart->layout()->setContentsMargins(0, 0, 0, 0);

  chartView = new QChartView(chart);
  chartView->setRenderHint(QPainter::Antialiasing);
  chartView->setBackgroundBrush(QBrush(QColor("#161616")));
  chartView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

  // Install event filter to capture events from the view
  chartView->installEventFilter(this);
  chartView->viewport()->installEventFilter(this);

  // Layout creation moved to constructor


  // --- 1. CANDLESTICK SERIES ---
  series = new QCandlestickSeries();
  series->setName("BTC");
  series->setIncreasingColor(QColor("#089981")); // Teal
  series->setDecreasingColor(QColor("#f23645")); // Red
  series->setBodyOutlineVisible(false);
  chart->addSeries(series);

  // --- 2. MOVING AVERAGE SERIES (SMA 20) ---
  maSeries = new QLineSeries();
  maSeries->setName("SMA 20");
  QPen maPen(QColor("#2962ff")); // Blue
  maPen.setWidth(2);
  maSeries->setPen(maPen);
  chart->addSeries(maSeries);
  


  // --- VOLUME REMOVED TO PREVENT CRASH WITH QDATETIMEAXIS ---
  // QBarSeries requires QBarCategoryAxis, inconsistent with QDateTimeAxis for candles.

  // --- AXES ---
  auto axisFont = QFont("Segoe UI", 9);
  QColor gridColor("#2a2e39");
  QColor labelColor("#b2b5be");

  // X Axis (Date)
  axisX = new QDateTimeAxis();
  axisX->setFormat("dd-MM");
  axisX->setLabelsColor(labelColor);
  axisX->setLabelsFont(axisFont);
  axisX->setGridLineVisible(true);
  axisX->setGridLinePen(QPen(gridColor, 1, Qt::SolidLine));
  axisX->setLineVisible(false);
  chart->addAxis(axisX, Qt::AlignBottom);
  
  series->attachAxis(axisX);
  maSeries->attachAxis(axisX);

  // Y Axis (Price) - Right
  axisY = new QValueAxis();
  axisY->setLabelsColor(labelColor);
  axisY->setLabelsFont(axisFont);
  axisY->setGridLineVisible(true);
  axisY->setGridLinePen(QPen(gridColor, 1, Qt::SolidLine));
  axisY->setLineVisible(false);
  chart->addAxis(axisY, Qt::AlignRight);
  
  series->attachAxis(axisY);
  maSeries->attachAxis(axisY);

  // --- CROSSHAIR ---
  QPen crosshairPen(QColor("#787b86"), 1, Qt::DashLine);
  crosshairX = new QGraphicsLineItem();
  crosshairX->setPen(crosshairPen);
  crosshairX->setZValue(100); // On top
  chart->scene()->addItem(crosshairX);

  crosshairY = new QGraphicsLineItem();
  crosshairY->setPen(crosshairPen);
  crosshairY->setZValue(100);
  chart->scene()->addItem(crosshairY);
  
  crosshairX->setVisible(false);
  crosshairY->setVisible(false);

  // --- INFO LABEL (HUD) ---
  infoLabel = new QGraphicsTextItem();
  infoLabel->setDefaultTextColor(QColor("#d1d4dc"));
  infoLabel->setFont(QFont("Segoe UI", 10));
  infoLabel->setPos(10, 10); // Top-left
  infoLabel->setZValue(101); // Above crosshair
  chart->scene()->addItem(infoLabel);
}

bool ChartWidget::connectToDatabase() {
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName("backtest.db");

  if (!db.open()) {
    qDebug() << "Error: connection with database failed";
    return false;
  }
  return true;
}

void ChartWidget::loadData(const QString &symbol, const QString &interval) {
  if (!connectToDatabase()) {
    return;
  }

  QSqlQuery query;
  // Select Volume as well
  query.prepare("SELECT Timestamp, Open, High, Low, Close, Volume FROM StockData WHERE "
                "Symbol = :symbol ORDER BY Timestamp");
  query.bindValue(":symbol", symbol);

  if (!query.exec()) {
    qDebug() << "Query failed:" << query.lastError();
    return;
  }

  series->clear();
  maSeries->clear();
  // volumeSeries->clear(); // Removed
  
  qint64 minTimestamp = std::numeric_limits<qint64>::max();
  qint64 maxTimestamp = std::numeric_limits<qint64>::min();
  double minPrice = std::numeric_limits<double>::max();
  double maxPrice = std::numeric_limits<double>::min();
  double maxVolume = 0;

  QList<double> closes; // For SMA calculation

  while (query.next()) {
    QString timestampStr = query.value(0).toString().trimmed();
    
    // Aggressive sanitization: corrupted data has extra lines/spaces
    if (timestampStr.length() >= 19) {
        timestampStr = timestampStr.left(19);
    } else {
        qDebug() << "Skipping invalid timestamp length:" << timestampStr.length();
        continue; // Skip invalid rows
    }

    double open = query.value(1).toDouble();
    double high = query.value(2).toDouble();
    double low = query.value(3).toDouble();
    double close = query.value(4).toDouble();
    double volume = query.value(5).toDouble();

    qDebug() << "Row: " << timestampStr << " O: " << open << " H: " << high;

    QDateTime timestamp = QDateTime::fromString(timestampStr, "yyyy-MM-dd HH:mm:ss");
    
    // Fallback moved inside length check, though likely unnecessary if format is fixed
    if (!timestamp.isValid()) timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);

    if (timestamp.isValid()) {
      qint64 ts = timestamp.toMSecsSinceEpoch();
      
      // Candle
      QCandlestickSet *set = new QCandlestickSet(open, high, low, close, ts);
      series->append(set);

      // Volume (disabled - causes Qt assertion errors with QDateTimeAxis)
      // QBarSet *volSet = new QBarSet("Volume");
      // volSet->append(volume);
      // 
      // // Color based on price action
      // if (close >= open)
      //     volSet->setColor(QColor("#089981")); // Green
      // else
      //     volSet->setColor(QColor("#f23645")); // Red
      //     
      // volSet->setBorderColor(Qt::transparent);
      // volumeSeries->append(volSet);

      if (volume > maxVolume) maxVolume = volume;
      
      // SMA Calculation
      closes.append(close);
      if (closes.size() >= 20) {
          double sum = 0;
          for (int i = closes.size() - 20; i < closes.size(); ++i) sum += closes[i];
          maSeries->append(ts, sum / 20.0);
      }

      if (ts < minTimestamp) minTimestamp = ts;
      if (ts > maxTimestamp) maxTimestamp = ts;
      if (low < minPrice) minPrice = low;
      if (high > maxPrice) maxPrice = high;
    } else {
         qDebug() << "Invalid timestamp object for string:" << timestampStr;
    }
  }

  qDebug() << "Finished Data Loop. Count:" << series->count();
  qDebug() << "MinTS:" << minTimestamp << "MaxTS:" << maxTimestamp;
  qDebug() << "MinPrice:" << minPrice << "MaxPrice:" << maxPrice;
  qDebug() << "MaxVolume:" << maxVolume;

  if (series->count() > 0 && minTimestamp != std::numeric_limits<qint64>::max() && maxTimestamp != std::numeric_limits<qint64>::min()) {
    
    // Safety check for flat ranges
    if (minTimestamp >= maxTimestamp) {
        qDebug() << "Adjusting flat Time range";
        maxTimestamp = minTimestamp + 86400000; // Adds 1 day
    }
    
    if (minPrice >= maxPrice) {
        qDebug() << "Adjusting flat Price range";
        minPrice = minPrice * 0.95;
        maxPrice = (maxPrice == 0 ? 1 : maxPrice * 1.05);
    }

    if (maxVolume <= 0) {
        maxVolume = 100; // Default
    }

    qDebug() << "Setting AxisX Range...";
    axisX->setRange(QDateTime::fromMSecsSinceEpoch(minTimestamp),
                    QDateTime::fromMSecsSinceEpoch(maxTimestamp));
    
    // Sync RSI Axis
    rsiAxisX->setRange(QDateTime::fromMSecsSinceEpoch(minTimestamp),
                       QDateTime::fromMSecsSinceEpoch(maxTimestamp));
                       
    // Calculate RSI
    QList<qint64> timestamps;
    // We need to reconstruct the list of timestamps matching 'closes'
    // Since we appended 'closes' in the loop, let's verify if we tracked timestamps properly.
    // The current loop implementation pushes to 'closes' one by one.
    // We should capture timestamps in a list parallel to 'closes'.
    
    // Let's iterate series->sets() to get consistent data for RSI
    QList<double> closePricesForRsi;
    QList<qint64> timestampsForRsi;
    
    for (int i = 0; i < series->count(); ++i) {
        QCandlestickSet *set = series->sets().at(i);
        closePricesForRsi.append(set->close());
        timestampsForRsi.append((qint64)set->timestamp());
    }
    
    calculateRSI(closePricesForRsi, timestampsForRsi);
    
    // Update RSI limits lines (30/70)
    if (m_rsiUpperLimit && m_rsiLowerLimit) {
        m_rsiUpperLimit->clear();
        m_rsiLowerLimit->clear();
        m_rsiUpperLimit->append(minTimestamp, 70);
        m_rsiUpperLimit->append(maxTimestamp, 70);
        m_rsiLowerLimit->append(minTimestamp, 30);
        m_rsiLowerLimit->append(maxTimestamp, 30);
    }

    axisY->setRange(minPrice * 0.99, maxPrice * 1.01);
    
    // Volume logic removed
  } else {
      qDebug() << "Skipping setRange due to no valid data.";
  }
}

bool ChartWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == chartView || watched == chartView->viewport() || 
        watched == rsiChartView || watched == rsiChartView->viewport()) {
        
        QChartView *targetView = nullptr;
        QChart *targetChart = nullptr;
        
        if (watched == chartView || watched == chartView->viewport()) {
            targetView = chartView;
            targetChart = chart;
        } else {
            targetView = rsiChartView;
            targetChart = rsiChart;
        }

        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_lastMousePos = mouseEvent->pos();
                m_isDragging = true;
                
                QPointF scenePos = targetView->mapToScene(mouseEvent->pos());
                QPointF chartPos = targetChart->mapFromScene(scenePos);
                QRectF plotArea = targetChart->plotArea();
                
                if (plotArea.contains(chartPos)) {
                    m_dragMode = DragMode::Pan;
                    setCursor(Qt::ClosedHandCursor);
                } else {
                    bool inXAxis = (chartPos.x() >= plotArea.left() && chartPos.x() <= plotArea.right() && chartPos.y() > plotArea.bottom());
                    bool inYAxis = (chartPos.y() >= plotArea.top() && chartPos.y() <= plotArea.bottom() && (chartPos.x() > plotArea.right() || chartPos.x() < plotArea.left()));

                    if (inXAxis) {
                         m_dragMode = DragMode::ZoomX;
                         setCursor(Qt::SizeHorCursor);
                    } else if (inYAxis) {
                         // RSI Y-axis is fixed 0-100 usually, so maybe disable Y zoom on RSI
                         if (targetChart == rsiChart) {
                             m_dragMode = DragMode::None;
                         } else {
                             m_dragMode = DragMode::ZoomY;
                             setCursor(Qt::SizeVerCursor);
                         }
                    } else {
                         m_dragMode = DragMode::None;
                    }
                }
                return true;
            }
        } 
        else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_isDragging = false;
                m_dragMode = DragMode::None;
                setCursor(Qt::ArrowCursor);
                return true;
            }
        }
        else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            
            // Panning & Zooming Logic
            if (m_isDragging) {
                QPoint delta = mouseEvent->pos() - m_lastMousePos;
                
                if (m_dragMode == DragMode::Pan) {
                    // Scroll the chart under the mouse (horizontal)
                    targetChart->scroll(-delta.x(), 0);
                    
                    // Vertical pan only if on main chart
                    if (targetChart == chart) {
                        targetChart->scroll(0, delta.y());
                    }
                } 
                else if (m_dragMode == DragMode::ZoomX) {
                    double sensitivity = 0.005; 
                    double factor = std::pow(1.0 - sensitivity, delta.x());
                    
                    // Use the axis of the target chart
                    QDateTimeAxis *targetAxisX = (targetChart == chart) ? axisX : rsiAxisX;
                    if (targetAxisX) {
                        qint64 min = targetAxisX->min().toMSecsSinceEpoch();
                        qint64 max = targetAxisX->max().toMSecsSinceEpoch();
                        qint64 center = (min + max) / 2;
                        qint64 span = max - min;
                        qint64 newSpan = span * factor;
                        if (newSpan < 60000) newSpan = 60000;
                        targetAxisX->setRange(QDateTime::fromMSecsSinceEpoch(center - newSpan/2), 
                                              QDateTime::fromMSecsSinceEpoch(center + newSpan/2));
                    }
                } 
                else if (m_dragMode == DragMode::ZoomY) {
                    double sensitivity = 0.005;
                    double factor = std::pow(1.0 + sensitivity, delta.y());
                    
                    if (axisY && targetChart == chart) {
                        double min = axisY->min();
                        double max = axisY->max();
                        double center = (min + max) / 2;
                        double span = max - min;
                        double newSpan = span * factor;
                        if (newSpan < 0.0001) newSpan = 0.0001;
                        axisY->setRange(center - newSpan/2, center + newSpan/2);
                    }
                }
                m_lastMousePos = mouseEvent->pos();
            }

            // Crosshair & Info Logic (Update ONLY if on Main Chart for now)
            if (targetChart == chart) {
                QPointF scenePos = chartView->mapToScene(mouseEvent->pos());
                QPointF chartPos = chart->mapFromScene(scenePos);

                if (chart->plotArea().contains(chartPos)) {
                    crosshairX->setVisible(true);
                    crosshairY->setVisible(true);
                    
                    updateCrosshair(chartPos);

                    // Update Info Label
                    if (series->count() > 0) {
                         QPointF valuePoint = chart->mapToValue(chartPos);
                         qint64 timestamp = (qint64)valuePoint.x();
                         
                         QCandlestickSet *closestSet = nullptr;
                         qint64 minDiff = std::numeric_limits<qint64>::max();

                         for (auto set : series->sets()) {
                             qint64 diff = std::abs(set->timestamp() - timestamp);
                             if (diff < minDiff) {
                                 minDiff = diff;
                                 closestSet = set;
                             }
                         }

                         if (closestSet && minDiff < 86400000) {
                             QString info = QString("BTC | %1 | O: %2 | H: %3 | L: %4 | C: %5")
                                                .arg(QDateTime::fromMSecsSinceEpoch(closestSet->timestamp()).toString("yyyy-MM-dd"))
                                                .arg(closestSet->open(), 0, 'f', 2)
                                                .arg(closestSet->high(), 0, 'f', 2)
                                                .arg(closestSet->low(), 0, 'f', 2)
                                                .arg(closestSet->close(), 0, 'f', 2);
                             infoLabel->setPlainText(info);
                             if (closestSet->close() >= closestSet->open()) {
                                  infoLabel->setDefaultTextColor(QColor("#089981"));
                             } else {
                                  infoLabel->setDefaultTextColor(QColor("#f23645"));
                             }
                         }
                    }
                } else {
                    crosshairX->setVisible(false);
                    crosshairY->setVisible(false);
                }
            }
            return true;
        }
        else if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
            // Apply zoom to Main Chart (which syncs RSI)
            if (wheelEvent->angleDelta().y() > 0) {
                chart->zoomIn();
            } else {
                chart->zoomOut();
            }
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ChartWidget::wheelEvent(QWheelEvent *event) {
    QWidget::wheelEvent(event);
}

void ChartWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);
}

void ChartWidget::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);
}

void ChartWidget::mouseMoveEvent(QMouseEvent *event) {
    QWidget::mouseMoveEvent(event);
}

void ChartWidget::enterEvent(QEnterEvent *event) {
    QWidget::enterEvent(event);
}

void ChartWidget::leaveEvent(QEvent *event) {
    crosshairX->setVisible(false);
    crosshairY->setVisible(false);
    QWidget::leaveEvent(event);
}

void ChartWidget::updateCrosshair(const QPointF &pos) {
    QRectF plotArea = chart->plotArea();
    
    QLineF vLine(pos.x(), plotArea.top(), pos.x(), plotArea.bottom());
    QLineF hLine(plotArea.left(), pos.y(), plotArea.right(), pos.y());
    
    // Safety check for invalid lines
    if (vLine.length() > 0 && hLine.length() > 0) {
        crosshairX->setLine(vLine);
        crosshairY->setLine(hLine);
    }
}

void ChartWidget::setupRsiChart() {
    rsiChart = new QChart();
    rsiChart->setTitle("");
    rsiChart->setBackgroundBrush(Qt::NoBrush);
    rsiChart->setPlotAreaBackgroundBrush(Qt::NoBrush);
    rsiChart->setPlotAreaBackgroundVisible(false);
    rsiChart->legend()->setVisible(false);
    rsiChart->setMargins(QMargins(0, 0, 0, 0));
    rsiChart->layout()->setContentsMargins(0, 0, 0, 0);

    rsiChartView = new QChartView(rsiChart);
    rsiChartView->setRenderHint(QPainter::Antialiasing);
    rsiChartView->setBackgroundBrush(QBrush(QColor("#161616")));

    // Install event filter for RSI interaction
    rsiChartView->installEventFilter(this);
    rsiChartView->viewport()->installEventFilter(this);
    
    // Series
    rsiSeries = new QLineSeries();
    rsiSeries->setName("RSI");
    QPen rsiPen(QColor("#7e57c2")); 
    rsiPen.setWidth(2);
    rsiSeries->setPen(rsiPen);
    rsiChart->addSeries(rsiSeries);
    
    // Axes
    auto axisFont = QFont("Segoe UI", 9);
    QColor gridColor("#2a2e39");
    QColor labelColor("#b2b5be");

    rsiAxisX = new QDateTimeAxis();
    rsiAxisX->setFormat("dd-MM");
    rsiAxisX->setLabelsVisible(false); // Hide X labels for RSI as it aligns with main chart
    rsiAxisX->setGridLineVisible(true);
    rsiAxisX->setGridLinePen(QPen(gridColor, 1, Qt::SolidLine));
    rsiAxisX->setLineVisible(false);
    rsiChart->addAxis(rsiAxisX, Qt::AlignBottom);
    rsiSeries->attachAxis(rsiAxisX);

    rsiAxisY = new QValueAxis();
    rsiAxisY->setRange(0, 100);
    rsiAxisY->setTickCount(3); // 0, 50, 100
    rsiAxisY->setLabelFormat("%.0f");
    rsiAxisY->setLabelsColor(labelColor);
    rsiAxisY->setLabelsFont(axisFont);
    rsiAxisY->setGridLineVisible(false); // Keep RSI Y grid disabled, use limit lines instead
    rsiAxisY->setLineVisible(false);
    rsiChart->addAxis(rsiAxisY, Qt::AlignRight);
    rsiSeries->attachAxis(rsiAxisY);

    // Add 30 and 70 limit lines
    QPen limitPen(QColor("#787b86"), 1, Qt::DashLine);
    
    m_rsiUpperLimit = new QLineSeries();
    m_rsiUpperLimit->setPen(limitPen);
    rsiChart->addSeries(m_rsiUpperLimit);
    m_rsiUpperLimit->attachAxis(rsiAxisX);
    m_rsiUpperLimit->attachAxis(rsiAxisY);
    
    m_rsiLowerLimit = new QLineSeries();
    m_rsiLowerLimit->setPen(limitPen);
    rsiChart->addSeries(m_rsiLowerLimit);
    m_rsiLowerLimit->attachAxis(rsiAxisX);
    m_rsiLowerLimit->attachAxis(rsiAxisY);

    // Bidirectional X Axis Synchronization
    connect(axisX, &QDateTimeAxis::rangeChanged, this, &ChartWidget::syncRsiToMain);
    connect(rsiAxisX, &QDateTimeAxis::rangeChanged, this, &ChartWidget::syncMainToRsi);
}

void ChartWidget::syncRsiToMain() {
    if (!rsiAxisX || !axisX) return;
    // Prevent infinite loop: only update if different
    if (rsiAxisX->min() != axisX->min() || rsiAxisX->max() != axisX->max()) {
        rsiAxisX->blockSignals(true);
        rsiAxisX->setRange(axisX->min(), axisX->max());
        rsiAxisX->blockSignals(false);
    }
}

void ChartWidget::syncMainToRsi() {
    if (!axisX || !rsiAxisX) return;
    // Prevent infinite loop: only update if different
    if (axisX->min() != rsiAxisX->min() || axisX->max() != rsiAxisX->max()) {
        axisX->blockSignals(true);
        axisX->setRange(rsiAxisX->min(), rsiAxisX->max());
        axisX->blockSignals(false);
    }
}

void ChartWidget::calculateRSI(const QList<double> &closePrices, const QList<qint64> &timestamps, int period) {
    rsiSeries->clear();
    
    if (closePrices.size() < period + 1) return;

    QList<double> gains;
    QList<double> losses;
    
    // Initial calculation
    double avgGain = 0;
    double avgLoss = 0;

    for (int i = 1; i <= period; ++i) {
        double change = closePrices[i] - closePrices[i - 1];
        if (change > 0) avgGain += change;
        else avgLoss += std::abs(change);
    }
    
    avgGain /= period;
    avgLoss /= period;

    // RSI Formula: RSI = 100 - (100 / (1 + RS))
    // RS = Average Gain / Average Loss

    for (int i = period; i < closePrices.size(); ++i) {
        double rs = (avgLoss == 0) ? 100 : (avgGain / avgLoss);
        double rsi = (avgLoss == 0) ? 100 : (100 - (100 / (1 + rs)));
        
        rsiSeries->append(timestamps[i], rsi);

        if (i < closePrices.size() - 1) {
             double change = closePrices[i + 1] - closePrices[i];
             double gain = (change > 0) ? change : 0;
             double loss = (change < 0) ? std::abs(change) : 0;
             
             avgGain = (avgGain * (period - 1) + gain) / period;
             avgLoss = (avgLoss * (period - 1) + loss) / period;
        }
    }
}

