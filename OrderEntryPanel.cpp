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
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    // ========== HEADER SECTION ==========
    // Row 1: Cross / Leverage buttons
    QHBoxLayout *leverageRow = new QHBoxLayout();
    leverageRow->setSpacing(8);

    m_crossBtn = new QPushButton("Cross");
    m_crossBtn->setCheckable(true);
    m_crossBtn->setChecked(true);
    m_crossBtn->setFixedHeight(32);

    m_isolatedBtn = new QPushButton("Isolated");
    m_isolatedBtn->setCheckable(true);
    m_isolatedBtn->setFixedHeight(32);

    QButtonGroup *marginGroup = new QButtonGroup(this);
    marginGroup->addButton(m_crossBtn);
    marginGroup->addButton(m_isolatedBtn);
    marginGroup->setExclusive(true);

    leverageRow->addWidget(m_crossBtn);
    leverageRow->addWidget(m_isolatedBtn);
    mainLayout->addLayout(leverageRow);

    // Row 2: Market / Limit tabs
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
    m_availableValue->setText("16.15 USDC");

    mainLayout->addWidget(createInfoRow("Current Position", &m_positionValue));
    m_positionValue->setText("0 GMT");

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
    m_unitCombo->addItems({"GMT", "USDC"});
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
    m_sizeSlider->setValue(11);

    m_sliderPercent = new QLabel("11 %");
    m_sliderPercent->setFixedWidth(50);
    m_sliderPercent->setAlignment(Qt::AlignCenter);

    connect(m_sizeSlider, &QSlider::valueChanged, this, &OrderEntryPanel::onSliderValueChanged);

    sliderRow->addWidget(m_sizeSlider, 1);
    sliderRow->addWidget(m_sliderPercent);
    mainLayout->addLayout(sliderRow);

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

    // ========== SPACER ==========
    mainLayout->addStretch();

    // ========== PLACE ORDER BUTTON ==========
    m_placeOrderBtn = new QPushButton("Place Order");
    m_placeOrderBtn->setFixedHeight(44);
    mainLayout->addWidget(m_placeOrderBtn);

    // ========== FOOTER SUMMARY ==========
    mainLayout->addWidget(createInfoRow("Liquidation Price", &m_liqPriceValue));
    m_liqPriceValue->setText("N/A");

    mainLayout->addWidget(createInfoRow("Order Value", &m_orderValueValue));
    m_orderValueValue->setText("5.31 USDC");

    mainLayout->addWidget(createInfoRow("Margin Required", &m_marginValue));
    m_marginValue->setText("1.77 USDC");


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

void OrderEntryPanel::onSliderValueChanged(int value) {
    m_sliderPercent->setText(QString("%1 %").arg(value));
}

void OrderEntryPanel::onTpSlToggled(bool checked) {
    m_tpSlContainer->setVisible(checked);
}

void OrderEntryPanel::updateTheme() {
    QString accentColor = (m_currentSide == Buy) ? COLOR_BUY : COLOR_SELL;
    QString inactiveBtn = QString("background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; font-weight: bold;")
                          .arg(COLOR_INPUT_BG, COLOR_TEXT_DIM, COLOR_BORDER);

    // Header buttons
    QString headerBtnStyle = QString(
        "QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; font-weight: bold; }"
        "QPushButton:checked { border-color: %4; color: %5; }"
    ).arg(COLOR_INPUT_BG, COLOR_TEXT_DIM, COLOR_BORDER, accentColor, COLOR_TEXT);
    m_crossBtn->setStyleSheet(headerBtnStyle);
    m_isolatedBtn->setStyleSheet(headerBtnStyle);

    // Tab buttons
    QString tabStyle = QString(
        "QPushButton { background: transparent; color: %1; border: none; border-bottom: 2px solid transparent; font-weight: bold; }"
        "QPushButton:checked { color: %2; border-bottom: 2px solid %2; }"
    ).arg(COLOR_TEXT_DIM, COLOR_TEXT);
    m_marketTab->setStyleSheet(tabStyle);
    m_limitTab->setStyleSheet(tabStyle);

    // Buy/Sell buttons
    if (m_currentSide == Buy) {
        m_buyBtn->setStyleSheet(QString("background-color: %1; color: %2; border: none; border-radius: 4px; font-weight: bold;").arg(COLOR_BUY, COLOR_TEXT));
        m_sellBtn->setStyleSheet(inactiveBtn);
    } else {
        m_sellBtn->setStyleSheet(QString("background-color: %1; color: %2; border: none; border-radius: 4px; font-weight: bold;").arg(COLOR_SELL, COLOR_TEXT));
        m_buyBtn->setStyleSheet(inactiveBtn);
    }

    // Place Order button
    m_placeOrderBtn->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border: none; border-radius: 6px; font-size: 14px; font-weight: bold; }"
        "QPushButton:hover { opacity: 0.9; }"
    ).arg(accentColor, COLOR_TEXT));

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
