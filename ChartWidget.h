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

  // Panning state
  bool m_isDragging = false;
  QPoint m_lastMousePos;

  void setupChart();
  bool connectToDatabase();
  void updateCrosshair(const QPointF &point);
};

#endif // CHARTWIDGET_H
