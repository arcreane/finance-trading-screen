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

private slots:
    void onMarketTabClicked();
    void onLimitTabClicked();
    void onBuyClicked();
    void onSellClicked();
    void onSliderValueChanged(int value);
    void onTpSlToggled(bool checked);
    void updateTheme();

private:
    enum OrderMode { Market, Limit };
    enum OrderSide { Buy, Sell };

    OrderMode m_currentMode = Market;
    OrderSide m_currentSide = Buy;

    // Header buttons
    QPushButton *m_crossBtn;
    QPushButton *m_isolatedBtn;
    QPushButton *m_marketTab;
    QPushButton *m_limitTab;

    // Buy/Sell buttons
    QPushButton *m_buyBtn;
    QPushButton *m_sellBtn;

    // Info labels
    QLabel *m_availableValue;
    QLabel *m_positionValue;

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
    QLabel *m_liqPriceValue;
    QLabel *m_orderValueValue;
    QLabel *m_marginValue;


    void setupUI();
    void setupStyle();
    QWidget* createInfoRow(const QString &label, QLabel **valueLabel);
    QWidget* createInputRow(const QString &label, QLineEdit **input, const QString &suffix = "");
};

#endif // ORDERENTRYPANEL_H
