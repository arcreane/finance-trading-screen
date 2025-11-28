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

private:
  QChartView *chartView;
  QChart *chart;
  QCandlestickSeries *series;
  QDateTimeAxis *axisX;
  QValueAxis *axisY;

  void setupChart();
  bool connectToDatabase();
};

#endif // CHARTWIDGET_H
