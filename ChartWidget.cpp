#include "ChartWidget.h"
#include <QDebug>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QGraphicsLayout>
#include <cmath>

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent) {
  setupChart();

  // Enable mouse tracking for crosshair
  setMouseTracking(true);
  chartView->setMouseTracking(true);

  // Auto-load BTC data as requested
  loadData("BTC", "Daily");
}

ChartWidget::~ChartWidget() {}

void ChartWidget::setupChart() {
  chart = new QChart();
  chart->setTitle("");
  
  // Background Colors
  chart->setBackgroundBrush(QBrush(QColor("#161616")));
  chart->setPlotAreaBackgroundBrush(QBrush(QColor("#161616")));
  chart->setPlotAreaBackgroundVisible(true);
  
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

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(chartView);

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
  axisX->setGridLineColor(gridColor);
  axisX->setLineVisible(false);
  chart->addAxis(axisX, Qt::AlignBottom);
  
  series->attachAxis(axisX);
  maSeries->attachAxis(axisX);

  // Y Axis (Price) - Right
  axisY = new QValueAxis();
  axisY->setLabelsColor(labelColor);
  axisY->setLabelsFont(axisFont);
  axisY->setGridLineColor(gridColor);
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
    qDebug() << "AxisX Set.";

    qDebug() << "Setting AxisY Range...";
    axisY->setRange(minPrice * 0.99, maxPrice * 1.01);
    qDebug() << "AxisY Set.";
    
    // Volume logic removed
  } else {
      qDebug() << "Skipping setRange due to no valid data.";
  }
}

bool ChartWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == chartView || watched == chartView->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_lastMousePos = mouseEvent->pos();
                m_isDragging = true;
                
                QPointF scenePos = chartView->mapToScene(mouseEvent->pos());
                QPointF chartPos = chart->mapFromScene(scenePos);
                QRectF plotArea = chart->plotArea();
                
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
                         m_dragMode = DragMode::ZoomY;
                         setCursor(Qt::SizeVerCursor);
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
                    chart->scroll(-delta.x(), delta.y());
                } 
                else if (m_dragMode == DragMode::ZoomX) {
                    double sensitivity = 0.005; 
                    double factor = std::pow(1.0 - sensitivity, delta.x());
                    
                    if (axisX) {
                        qint64 min = axisX->min().toMSecsSinceEpoch();
                        qint64 max = axisX->max().toMSecsSinceEpoch();
                        qint64 center = (min + max) / 2;
                        qint64 span = max - min;
                        qint64 newSpan = span * factor;
                        if (newSpan < 60000) newSpan = 60000;
                        axisX->setRange(QDateTime::fromMSecsSinceEpoch(center - newSpan/2), 
                                        QDateTime::fromMSecsSinceEpoch(center + newSpan/2));
                    }
                } 
                else if (m_dragMode == DragMode::ZoomY) {
                    double sensitivity = 0.005;
                    double factor = std::pow(1.0 + sensitivity, delta.y());
                    
                    if (axisY) {
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

            // Crosshair & Info Logic
            // Map strictly to plot area
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
            return true;
        }
        else if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
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