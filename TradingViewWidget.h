#pragma once
#include <QWidget>

class TradingViewWidget : public QWidget {
    Q_OBJECT
public:
    explicit TradingViewWidget(QWidget* parent = nullptr) : QWidget(parent) {}
};
