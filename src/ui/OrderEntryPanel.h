#ifndef ORDERENTRYPANEL_H
#define ORDERENTRYPANEL_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QButtonGroup>

class OrderEntryPanel : public QWidget {
    Q_OBJECT

public:
    explicit OrderEntryPanel(QWidget *parent = nullptr);

signals:
    void orderPlaced(const QString &symbol, const QString &type, const QString &side, const QString &price, const QString &amount);
    void balanceUpdated(double balance);

public slots:
    void setSymbol(const QString &symbol);
    void setCurrentPrice(double price);
    void refundBalance(double refundAmount);

private slots:
    void onMarketTabClicked();
    void onLimitTabClicked();
    void onBuyClicked();
    void onSellClicked();
    void onPlaceOrderClicked();
    void onSliderValueChanged(int value);
    void onTpSlToggled(bool checked);
    void onTpGainEdited(const QString &text);
    void onSlLossEdited(const QString &text);
    void updateTheme();

private:
    enum OrderMode { Market, Limit };
    enum OrderSide { Buy, Sell };

    OrderMode m_currentMode = Market;
    OrderSide m_currentSide = Buy;
    QString m_symbol = "BTC";
    double m_currentMarketPrice = 96000.0;

    // Header buttons
    QPushButton *m_marketTab;
    QPushButton *m_limitTab;

    // Buy/Sell buttons
    QPushButton *m_buyBtn;
    QPushButton *m_sellBtn;

    // Info labels
    QLabel *m_availableValue;

    // Form inputs
    QWidget *m_priceContainer;
    QLineEdit *m_priceInput;
    QLineEdit *m_sizeInput;
    QComboBox *m_unitCombo;

    // Slider
    QSlider *m_sizeSlider;
    QLabel *m_sliderPercent;

    // TP/SL
    QCheckBox *m_tpSlCheck;
    QWidget *m_tpSlContainer;
    QLineEdit *m_tpPriceInput;
    QLineEdit *m_tpGainInput;
    QLineEdit *m_slPriceInput;
    QLineEdit *m_slLossInput;

    // Footer
    QPushButton *m_placeOrderBtn;


    void setupUI();
    void setupStyle();
    QWidget* createInfoRow(const QString &label, QLabel **valueLabel);
    QWidget* createInputRow(const QString &label, QLineEdit **input, const QString &suffix = "");
};

#endif // ORDERENTRYPANEL_H
