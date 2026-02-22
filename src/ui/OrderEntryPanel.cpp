#include "OrderEntryPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QGridLayout>
#include <QPalette>

// Color constants
static const QString COLOR_BG = "#161616";
static const QString COLOR_INPUT_BG = "#1e1e1e";
static const QString COLOR_BORDER = "#333333";
static const QString COLOR_TEXT = "#ffffff";
static const QString COLOR_TEXT_DIM = "#888888";
static const QString COLOR_BUY = "#2db9b9";
static const QString COLOR_SELL = "#e24a6d";

OrderEntryPanel::OrderEntryPanel(QWidget *parent) : QWidget(parent) {
    setupUI();
    setupStyle();
    updateTheme();
}

void OrderEntryPanel::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(16); // Increased spacing for clearer interface

    // ========== HEADER SECTION ==========
    // Market / Limit tabs
    QHBoxLayout *tabRow = new QHBoxLayout();
    tabRow->setSpacing(10);

    m_marketTab = new QPushButton("Market");
    m_marketTab->setCheckable(true);
    m_marketTab->setChecked(true);
    m_marketTab->setFixedHeight(36);

    m_limitTab = new QPushButton("Limit");
    m_limitTab->setCheckable(true);
    m_limitTab->setFixedHeight(36);

    connect(m_marketTab, &QPushButton::clicked, this, &OrderEntryPanel::onMarketTabClicked);
    connect(m_limitTab, &QPushButton::clicked, this, &OrderEntryPanel::onLimitTabClicked);

    tabRow->addWidget(m_marketTab);
    tabRow->addWidget(m_limitTab);
    tabRow->addStretch();
    mainLayout->addLayout(tabRow);

    // ========== BUY/SELL SECTION ==========
    QHBoxLayout *buySellRow = new QHBoxLayout();
    buySellRow->setSpacing(4);

    m_buyBtn = new QPushButton("Buy / Long");
    m_buyBtn->setCheckable(true);
    m_buyBtn->setChecked(true);
    m_buyBtn->setFixedHeight(40);

    m_sellBtn = new QPushButton("Sell / Short");
    m_sellBtn->setCheckable(true);
    m_sellBtn->setFixedHeight(40);

    connect(m_buyBtn, &QPushButton::clicked, this, &OrderEntryPanel::onBuyClicked);
    connect(m_sellBtn, &QPushButton::clicked, this, &OrderEntryPanel::onSellClicked);

    buySellRow->addWidget(m_buyBtn, 1);
    buySellRow->addWidget(m_sellBtn, 1);
    mainLayout->addLayout(buySellRow);

    // ========== INFO SECTION ==========
    mainLayout->addWidget(createInfoRow("Available to Trade", &m_availableValue));
    m_availableValue->setText("100.00 USDC");
    
    // Defer emitting until connected
    QMetaObject::invokeMethod(this, [this]() {
        emit balanceUpdated(100.0);
    }, Qt::QueuedConnection);



    // ========== PRICE INPUT (Limit mode only) ==========
    m_priceContainer = new QWidget();
    QHBoxLayout *priceLayout = new QHBoxLayout(m_priceContainer);
    priceLayout->setContentsMargins(0, 0, 0, 0);
    priceLayout->setSpacing(0);

    QLabel *priceLabel = new QLabel("Price");
    priceLabel->setStyleSheet(QString("color: %1; padding: 8px;").arg(COLOR_TEXT_DIM));

    m_priceInput = new QLineEdit();
    m_priceInput->setPlaceholderText("0.00");
    m_priceInput->setAlignment(Qt::AlignRight);

    QLabel *priceSuffix = new QLabel("USDC");
    priceSuffix->setStyleSheet(QString("color: %1; padding: 8px;").arg(COLOR_TEXT_DIM));

    priceLayout->addWidget(priceLabel);
    priceLayout->addWidget(m_priceInput, 1);
    priceLayout->addWidget(priceSuffix);

    m_priceContainer->hide(); // Hidden by default (Market mode)
    mainLayout->addWidget(m_priceContainer);

    // ========== SIZE INPUT ==========
    QWidget *sizeContainer = new QWidget();
    QHBoxLayout *sizeLayout = new QHBoxLayout(sizeContainer);
    sizeLayout->setContentsMargins(0, 0, 0, 0);
    sizeLayout->setSpacing(0);

    QLabel *sizeLabel = new QLabel("Size");
    sizeLabel->setStyleSheet(QString("color: %1; padding: 8px;").arg(COLOR_TEXT_DIM));

    m_sizeInput = new QLineEdit();
    m_sizeInput->setPlaceholderText("0");
    m_sizeInput->setAlignment(Qt::AlignRight);

    m_unitCombo = new QComboBox();
    m_unitCombo->addItems({m_symbol, "USDC"});
    m_unitCombo->setFixedWidth(80);

    sizeLayout->addWidget(sizeLabel);
    sizeLayout->addWidget(m_sizeInput, 1);
    sizeLayout->addWidget(m_unitCombo);

    mainLayout->addWidget(sizeContainer);

    // ========== SLIDER ==========
    QHBoxLayout *sliderRow = new QHBoxLayout();
    sliderRow->setSpacing(8);

    m_sizeSlider = new QSlider(Qt::Horizontal);
    m_sizeSlider->setRange(0, 100);
    m_sizeSlider->blockSignals(true);
    m_sizeSlider->setValue(0);
    m_sizeSlider->blockSignals(false);

    m_sliderPercent = new QLabel("0 %");
    m_sliderPercent->setFixedWidth(50);
    m_sliderPercent->setAlignment(Qt::AlignCenter);

    connect(m_sizeSlider, &QSlider::valueChanged, this, &OrderEntryPanel::onSliderValueChanged);

    sliderRow->addWidget(m_sizeSlider, 1);
    sliderRow->addWidget(m_sliderPercent);
    mainLayout->addLayout(sliderRow);

    connect(m_sizeInput, &QLineEdit::textEdited, this, [this](const QString &text) {
        QString availText = m_availableValue->text();
        double avail = availText.split(" ").first().toDouble();
        if (avail <= 0.0) return;

        double enteredSize = text.toDouble();
        if (m_unitCombo->currentText() != "USDC") {
            double price = 0.0;
            if (m_currentMode == Limit) {
                price = m_priceInput->text().toDouble();
            }
            if (price <= 0.0) price = m_currentMarketPrice;
            enteredSize *= price;
        }

        int percent = qBound(0, static_cast<int>((enteredSize / avail) * 100.0), 100);
        
        m_sizeSlider->blockSignals(true);
        m_sizeSlider->setValue(percent);
        m_sliderPercent->setText(QString("%1 %").arg(percent));
        m_sizeSlider->blockSignals(false);
    });

    // ========== TP/SL SECTION ==========
    m_tpSlCheck = new QCheckBox("Take Profit / Stop Loss");
    m_tpSlCheck->setChecked(true);
    connect(m_tpSlCheck, &QCheckBox::toggled, this, &OrderEntryPanel::onTpSlToggled);
    mainLayout->addWidget(m_tpSlCheck);

    m_tpSlContainer = new QWidget();
    QGridLayout *tpSlGrid = new QGridLayout(m_tpSlContainer);
    tpSlGrid->setContentsMargins(0, 0, 0, 0);
    tpSlGrid->setSpacing(8);

    // TP Price
    m_tpPriceInput = new QLineEdit();
    m_tpPriceInput->setPlaceholderText("TP Price");

    // Gain %
    m_tpGainInput = new QLineEdit();
    m_tpGainInput->setPlaceholderText("Gain");
    connect(m_tpGainInput, &QLineEdit::textEdited, this, &OrderEntryPanel::onTpGainEdited);

    QLabel *gainSuffix = new QLabel("%");
    gainSuffix->setStyleSheet(QString("color: %1;").arg(COLOR_TEXT_DIM));

    QHBoxLayout *gainRow = new QHBoxLayout();
    gainRow->setSpacing(4);
    gainRow->addWidget(m_tpGainInput, 1);
    gainRow->addWidget(gainSuffix);

    // SL Price
    m_slPriceInput = new QLineEdit();
    m_slPriceInput->setPlaceholderText("SL Price");

    // Loss %
    m_slLossInput = new QLineEdit();
    m_slLossInput->setPlaceholderText("Loss");
    connect(m_slLossInput, &QLineEdit::textEdited, this, &OrderEntryPanel::onSlLossEdited);

    QLabel *lossSuffix = new QLabel("%");
    lossSuffix->setStyleSheet(QString("color: %1;").arg(COLOR_TEXT_DIM));

    QHBoxLayout *lossRow = new QHBoxLayout();
    lossRow->setSpacing(4);
    lossRow->addWidget(m_slLossInput, 1);
    lossRow->addWidget(lossSuffix);

    tpSlGrid->addWidget(m_tpPriceInput, 0, 0);
    tpSlGrid->addLayout(gainRow, 0, 1);
    tpSlGrid->addWidget(m_slPriceInput, 1, 0);
    tpSlGrid->addLayout(lossRow, 1, 1);

    mainLayout->addWidget(m_tpSlContainer);

    // ========== PLACE ORDER BUTTON ==========
    m_placeOrderBtn = new QPushButton("Place Order");
    m_placeOrderBtn->setFixedHeight(44);
    connect(m_placeOrderBtn, &QPushButton::clicked, this, &OrderEntryPanel::onPlaceOrderClicked);
    mainLayout->addWidget(m_placeOrderBtn);

    // ========== FOOTER SUMMARY ==========



    // ========== SPACER (fills remaining space at bottom) ==========
    mainLayout->addStretch();

}

QWidget* OrderEntryPanel::createInfoRow(const QString &label, QLabel **valueLabel) {
    QWidget *row = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 2, 0, 2);
    layout->setSpacing(0);

    QLabel *labelWidget = new QLabel(label);
    labelWidget->setStyleSheet(QString("color: %1;").arg(COLOR_TEXT_DIM));

    *valueLabel = new QLabel();
    (*valueLabel)->setAlignment(Qt::AlignRight);
    (*valueLabel)->setStyleSheet(QString("color: %1;").arg(COLOR_TEXT));

    layout->addWidget(labelWidget);
    layout->addStretch();
    layout->addWidget(*valueLabel);

    return row;
}

void OrderEntryPanel::setupStyle() {
    setStyleSheet(QString(
        "QWidget { background-color: %1; color: %2; font-family: 'Segoe UI', sans-serif; font-size: 12px; }"
        "QLineEdit { background-color: %3; border: 1px solid %4; border-radius: 4px; padding: 8px; color: %2; }"
        "QLineEdit:focus { border-color: %5; }"
        "QComboBox { background-color: %3; border: 1px solid %4; border-radius: 4px; padding: 6px; color: %2; }"
        "QComboBox::drop-down { border: none; width: 20px; }"
        "QComboBox::down-arrow { image: none; border-left: 4px solid transparent; border-right: 4px solid transparent; border-top: 5px solid %6; }"
        "QComboBox QAbstractItemView { background-color: %3; color: %2; selection-background-color: %4; }"
        "QCheckBox { color: %2; spacing: 8px; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid %4; border-radius: 3px; background: %3; }"
        "QCheckBox::indicator:checked { background-color: %5; border-color: %5; }"
        "QSlider::groove:horizontal { background: %4; height: 4px; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: %5; width: 14px; height: 14px; margin: -5px 0; border-radius: 7px; }"
    ).arg(COLOR_BG, COLOR_TEXT, COLOR_INPUT_BG, COLOR_BORDER, COLOR_BUY, COLOR_TEXT_DIM));

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(COLOR_BG));
    pal.setColor(QPalette::Base, QColor(COLOR_INPUT_BG));
    setPalette(pal);
    setAutoFillBackground(true);
}

void OrderEntryPanel::onMarketTabClicked() {
    m_currentMode = Market;
    m_marketTab->setChecked(true);
    m_limitTab->setChecked(false);
    m_priceContainer->hide();
    updateTheme();
}

void OrderEntryPanel::onLimitTabClicked() {
    m_currentMode = Limit;
    m_marketTab->setChecked(false);
    m_limitTab->setChecked(true);
    m_priceContainer->show();
    updateTheme();
}

void OrderEntryPanel::onBuyClicked() {
    m_currentSide = Buy;
    m_buyBtn->setChecked(true);
    m_sellBtn->setChecked(false);
    updateTheme();
}

void OrderEntryPanel::onSellClicked() {
    m_currentSide = Sell;
    m_buyBtn->setChecked(false);
    m_sellBtn->setChecked(true);
    updateTheme();
}

void OrderEntryPanel::onPlaceOrderClicked() {
    double size = m_sizeInput->text().toDouble();
    if (size <= 0.0) return;

    QString typeStr = (m_currentMode == Market) ? "Market" : "Limit";
    double price = 0.0;
    QString priceStr = "Market";

    if (m_currentMode == Limit) {
        price = m_priceInput->text().toDouble();
        if (price <= 0.0) return;
        priceStr = QString::number(price, 'f', 2);
    } else {
        price = m_currentMarketPrice;
        priceStr = QString::number(price, 'f', 2);
    }

    double costUsdc = 0.0;
    QString amountStr;

    if (m_unitCombo->currentText() == "USDC") {
        costUsdc = size;
        double tokenAmount = size / price;
        amountStr = QString("%1 %2").arg(QString::number(tokenAmount, 'f', 5), m_symbol);
    } else {
        costUsdc = size * price;
        amountStr = QString("%1 %2").arg(QString::number(size, 'f', 5), m_symbol);
    }

    QString availText = m_availableValue->text();
    double avail = availText.split(" ").first().toDouble();

    if (avail >= costUsdc) {
        avail -= costUsdc;
        m_availableValue->setText(QString("%1 USDC").arg(avail, 0, 'f', 2));
        emit balanceUpdated(avail);

        QString sideStr = (m_currentSide == Buy) ? "Buy" : "Sell";
        emit orderPlaced(m_symbol, typeStr, sideStr, priceStr, amountStr);

        m_sizeInput->clear();
        m_sizeSlider->blockSignals(true);
        m_sizeSlider->setValue(0);
        m_sizeSlider->blockSignals(false);
        m_sliderPercent->setText("0 %");
    }
}

void OrderEntryPanel::onSliderValueChanged(int value) {
    m_sliderPercent->setText(QString("%1 %").arg(value));

    QString availText = m_availableValue->text();
    double avail = availText.split(" ").first().toDouble();
    double sizeVal = avail * (value / 100.0);

    if (m_unitCombo->currentText() != "USDC") {
        double price = 0.0;
        if (m_currentMode == Limit) {
            price = m_priceInput->text().toDouble();
        }
        if (price <= 0.0) price = m_currentMarketPrice;
        sizeVal = sizeVal / price;
        m_sizeInput->setText(QString::number(sizeVal, 'f', 5));
    } else {
        m_sizeInput->setText(QString::number(sizeVal, 'f', 2));
    }
}

void OrderEntryPanel::onTpSlToggled(bool checked) {
    m_tpSlContainer->setVisible(checked);
}

void OrderEntryPanel::onTpGainEdited(const QString &text) {
    if (text.isEmpty()) {
        m_tpPriceInput->clear();
        return;
    }
    
    double percent = text.toDouble();
    if (percent == 0.0) return;
    
    double entryPrice = 0.0;
    if (m_currentMode == Limit) {
        entryPrice = m_priceInput->text().toDouble();
    }
    if (entryPrice <= 0.0) {
        entryPrice = m_currentMarketPrice;
    }
    
    double targetPrice = 0.0;
    if (m_currentSide == Buy) {
        targetPrice = entryPrice * (1.0 + (percent / 100.0));
    } else {
        targetPrice = entryPrice * (1.0 - (percent / 100.0));
    }
    
    if (targetPrice > 0.0) {
        m_tpPriceInput->setText(QString::number(targetPrice, 'f', 2));
    }
}

void OrderEntryPanel::onSlLossEdited(const QString &text) {
    if (text.isEmpty()) {
        m_slPriceInput->clear();
        return;
    }
    
    double percent = text.toDouble();
    if (percent == 0.0) return;
    
    double entryPrice = 0.0;
    if (m_currentMode == Limit) {
        entryPrice = m_priceInput->text().toDouble();
    }
    if (entryPrice <= 0.0) {
        entryPrice = m_currentMarketPrice;
    }
    
    double targetPrice = 0.0;
    if (m_currentSide == Buy) {
        targetPrice = entryPrice * (1.0 - (percent / 100.0));
    } else {
        targetPrice = entryPrice * (1.0 + (percent / 100.0));
    }
    
    if (targetPrice > 0.0) {
        m_slPriceInput->setText(QString::number(targetPrice, 'f', 2));
    }
}

void OrderEntryPanel::updateTheme() {
    QString accentColor = (m_currentSide == Buy) ? COLOR_BUY : COLOR_SELL;
    QString inactiveBtn = QString("background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; font-weight: bold;")
                          .arg(COLOR_INPUT_BG, COLOR_TEXT_DIM, COLOR_BORDER);

    // Tab buttons
    QString tabStyle = QString(
        "QPushButton { background: transparent; color: %1; border: none; border-bottom: 2px solid transparent; font-weight: bold; }"
        "QPushButton:checked { color: %2; border-bottom: 2px solid %2; }"
    ).arg(COLOR_TEXT_DIM, COLOR_TEXT);
    m_marketTab->setStyleSheet(tabStyle);
    m_limitTab->setStyleSheet(tabStyle);

    // Buy/Sell buttons
    if (m_currentSide == Buy) {
        m_buyBtn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: %2; border: none; border-radius: 4px; font-weight: bold; }"
            "QPushButton:pressed { background-color: #249494; /* Darker teal for pressed state */ }"
        ).arg(COLOR_BUY, COLOR_TEXT));
        m_sellBtn->setStyleSheet(inactiveBtn);
    } else {
        m_sellBtn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: %2; border: none; border-radius: 4px; font-weight: bold; }"
            "QPushButton:pressed { background-color: #b53b57; /* Darker red for pressed state */ }"
        ).arg(COLOR_SELL, COLOR_TEXT));
        m_buyBtn->setStyleSheet(inactiveBtn);
    }

    // Place Order button
    m_placeOrderBtn->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border: none; border-radius: 6px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { opacity: 0.9; }"
        "QPushButton:pressed { background-color: %3; }"
    ).arg(accentColor, COLOR_TEXT, (m_currentSide == Buy) ? "#249494" : "#b53b57"));

    // Slider
    QString sliderStyle = QString(
        "QSlider::groove:horizontal { background: %1; height: 4px; border-radius: 2px; }"
        "QSlider::sub-page:horizontal { background: %2; border-radius: 2px; }"
        "QSlider::handle:horizontal { background: %2; width: 14px; height: 14px; margin: -5px 0; border-radius: 7px; }"
    ).arg(COLOR_BORDER, accentColor);
    m_sizeSlider->setStyleSheet(sliderStyle);

    // Slider percent badge
    m_sliderPercent->setStyleSheet(QString(
        "background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 4px;"
    ).arg(COLOR_INPUT_BG, COLOR_TEXT, COLOR_BORDER));
}

void OrderEntryPanel::setSymbol(const QString &symbol) {
    if (m_symbol == symbol) return;
    m_symbol = symbol;
    m_unitCombo->setItemText(0, m_symbol);
}

void OrderEntryPanel::setCurrentPrice(double price) {
    m_currentMarketPrice = price;
}

void OrderEntryPanel::refundBalance(double refundAmount) {
    if (refundAmount <= 0) return;
    
    QString availText = m_availableValue->text();
    double avail = availText.split(" ").first().toDouble();
    avail += refundAmount;
    
    m_availableValue->setText(QString("%1 USDC").arg(avail, 0, 'f', 2));
    emit balanceUpdated(avail);
}


