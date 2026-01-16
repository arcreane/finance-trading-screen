#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <QtSql>

class ChartWidget : public QWidget {
  Q_OBJECT

public:
  explicit ChartWidget(QWidget *parent = nullptr);
  ~ChartWidget();

  void loadData(const QString &symbol, const QString &interval);
  
protected:
  // Crosshair & Panning events
  bool eventFilter(QObject *watched, QEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  QChartView *chartView;
  QChart *chart;
  QCandlestickSeries *series;
  // QBarSeries *volumeSeries; // Removed
  QLineSeries *maSeries;    // Moving Average
  
  QDateTimeAxis *axisX;
  QValueAxis *axisY;
  // QValueAxis *axisYVolume; // Removed

  // Crosshair items
  QGraphicsLineItem *crosshairX;
  QGraphicsLineItem *crosshairY;
  QGraphicsTextItem *priceLabel;
  QGraphicsTextItem *dateLabel;
  QGraphicsTextItem *infoLabel; // HUD for candle info

  // Panning & Zooming state
  enum class DragMode { None, Pan, ZoomX, ZoomY };
  DragMode m_dragMode = DragMode::None;
  bool m_isDragging = false;
  QPoint m_lastMousePos;

  // RSI Components
  QChartView *rsiChartView;
  QChart *rsiChart;
  QLineSeries *rsiSeries;
  QDateTimeAxis *rsiAxisX;
  QValueAxis *rsiAxisY;

  void setupChart();
  void setupRsiChart();
  bool connectToDatabase();
  void updateCrosshair(const QPointF &point);
  void calculateRSI(const QList<double> &closePrices, const QList<qint64> &timestamps, int period = 14);
  
  // Bidirectional axis sync slots
  void syncRsiToMain();
  void syncMainToRsi();

  // RSI Limit Lines (30 and 70)
  QLineSeries *m_rsiUpperLimit; // 70
  QLineSeries *m_rsiLowerLimit; // 30
};

#endif // CHARTWIDGET_H
