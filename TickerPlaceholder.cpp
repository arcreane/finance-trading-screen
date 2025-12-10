#include "TickerPlaceholder.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QEvent>
#include <QApplication>

// ==========================================
// 1. Implémentation du Selecteur (Le Popup)
// ==========================================

TickerSelector::TickerSelector(QWidget *parent) : QDialog(parent, Qt::Popup | Qt::FramelessWindowHint) {
    setupUI();
    loadDummyData();
}

void TickerSelector::setupUI() {
    this->setFixedSize(850, 500);
    // Fond très sombre et bordure légère
    this->setStyleSheet("background-color: #1b2028; border: 1px solid #2a2e39; border-radius: 8px;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    // -- Barre de recherche --
    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Search");
    searchEdit->setStyleSheet(
        "QLineEdit { background-color: #232832; border: 1px solid #2a2e39; padding: 10px; color: white; border-radius: 4px; font-size: 14px; }"
        "QLineEdit:focus { border: 1px solid #4a90e2; }"
    );
    layout->addWidget(searchEdit);

    // -- Tableau des tickers --
    tickerTable = new QTableWidget(this);
    tickerTable->setColumnCount(6);
    tickerTable->setHorizontalHeaderLabels({"Symbol", "Last Price", "24h Change", "8hr Funding", "Volume", "Open Interest"});
    
    // Configuration du tableau
    tickerTable->verticalHeader()->setVisible(false);
    tickerTable->setShowGrid(false);
    tickerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tickerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tickerTable->setFocusPolicy(Qt::NoFocus);
    
    // Style du tableau pour matcher Hyperliquid
    tickerTable->setStyleSheet(
        "QTableWidget { background-color: transparent; border: none; }"
        "QHeaderView::section { background-color: transparent; color: #848e9c; border: none; font-weight: bold; padding: 5px; text-align: left; }"
        "QTableWidget::item { padding: 8px; border-bottom: 1px solid #232832; color: #c3c5cb; }"
        "QTableWidget::item:selected { background-color: #2a2e39; }"
        "QTableWidget::item:hover { background-color: #232832; }"
    );

    tickerTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tickerTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    layout->addWidget(tickerTable);
}

void TickerSelector::loadDummyData() {
    // Données basées sur ta capture
    struct CoinData { QString sym; QString price; QString chg; QString fund; QString vol; QString oi; };
    QList<CoinData> data = {
        {"SOL-USDC", "136.72", "-3.64%", "0.0100%", "$304M", "$426M"},
        {"TSLA-USDC", "431.39", "+0.85%", "0.0693%", "$9.8M", "$8.0M"},
        {"SUI-USDC", "1.4988", "-3.85%", "0.0100%", "$7.6M", "$19M"},
        {"HYPE-USDC", "35.515", "+0.24%", "--", "$92.2M", "--"},
        {"BTC-USDC", "96500.00", "+1.20%", "0.0100%", "$1.2B", "$5.5B"}
    };

    tickerTable->setRowCount(data.size());

    for (int i = 0; i < data.size(); ++i) {
        // Colonne Symbol (avec une étoile)
        QTableWidgetItem *sym = new QTableWidgetItem("★  " + data[i].sym);
        sym->setForeground(QBrush(QColor("white")));
        tickerTable->setItem(i, 0, sym);

        // Price
        QTableWidgetItem *price = new QTableWidgetItem(data[i].price);
        price->setForeground(QBrush(QColor("white")));
        tickerTable->setItem(i, 1, price);

        // Change (Vert ou Rouge)
        QTableWidgetItem *chg = new QTableWidgetItem(data[i].chg);
        if (data[i].chg.contains("+")) chg->setForeground(QBrush(QColor("#0ecb81"))); // Vert
        else chg->setForeground(QBrush(QColor("#f6465d"))); // Rouge
        tickerTable->setItem(i, 2, chg);

        tickerTable->setItem(i, 3, new QTableWidgetItem(data[i].fund));
        tickerTable->setItem(i, 4, new QTableWidgetItem(data[i].vol));
        tickerTable->setItem(i, 5, new QTableWidgetItem(data[i].oi));
    }
}

// Gestion fermeture propre
bool TickerSelector::event(QEvent *event) {
    if (event->type() == QEvent::WindowDeactivate) {
        this->close();
    }
    return QDialog::event(event);
}


// ==========================================
// 2. Implémentation de la Barre Principale
// ==========================================

TickerPlaceholder::TickerPlaceholder(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void TickerPlaceholder::setupUI() {
    // Hauteur fixe comme dans ton code original (50px -> 60px pour aérer)
    this->setFixedHeight(60);
    // Fond sombre global pour cette barre
    this->setStyleSheet("background-color: #131722; border-bottom: 1px solid #2a2e39;");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 0, 15, 0);
    mainLayout->setSpacing(20);

    // --- SECTION GAUCHE : Selecteur ---
    QWidget *leftContainer = new QWidget();
    leftContainer->setStyleSheet("background: transparent; border: none;"); // Pas de bordure ici
    QHBoxLayout *leftLayout = new QHBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0,0,0,0);
    leftLayout->setSpacing(8);

    // Logo (rond vert)
    QLabel *logo = new QLabel();
    logo->setFixedSize(20, 20);
    logo->setStyleSheet("background-color: #7df2d5; border-radius: 10px; border: none;");

    // Bouton Symbole
    symbolButton = new QPushButton("HYPE/USDC");
    symbolButton->setCursor(Qt::PointingHandCursor);
    symbolButton->setStyleSheet(
        "QPushButton { color: white; font-size: 18px; font-weight: bold; border: none; background: transparent; text-align: left; }"
        "QPushButton:hover { color: #cccccc; }"
    );
    connect(symbolButton, &QPushButton::clicked, this, &TickerPlaceholder::openTickerSelector);

    // Flèche bas
    QLabel *arrow = new QLabel("v");
    arrow->setStyleSheet("color: #848e9c; font-weight: bold; padding-bottom: 3px; border: none;");

    // Badge Spot
    QLabel *badge = new QLabel("Spot");
    badge->setStyleSheet("background-color: #1e3a3a; color: #3BB3A3; padding: 2px 6px; border-radius: 4px; font-weight: bold; font-size: 11px; border: none;");

    leftLayout->addWidget(logo);
    leftLayout->addWidget(symbolButton);
    leftLayout->addWidget(arrow);
    leftLayout->addWidget(badge);

    mainLayout->addWidget(leftContainer);

    // Séparateur vertical
    QFrame *vLine = new QFrame;
    vLine->setFrameShape(QFrame::VLine);
    vLine->setStyleSheet("color: #2a2e39;");
    mainLayout->addWidget(vLine);

    // --- SECTION MILLIEU : Stats ---
    // Price
    mainLayout->addWidget(createStatWidget("Price", "35,515", "#f6465d")); // Rouge
    // Change
    mainLayout->addWidget(createStatWidget("24h Change", "+0,084 / +0,24%", "#0ecb81")); // Vert
    // Volume
    mainLayout->addWidget(createStatWidget("24h Volume", "92 218 791,07 USDC", "white"));
    // Cap
    mainLayout->addWidget(createStatWidget("Market Cap", "12 043 025 498 USDC", "white"));

    // Stretch pour pousser le reste à droite
    mainLayout->addStretch();

    // --- SECTION DROITE : Contract ---
    QLabel *contract = new QLabel("Contract 0x0d01...11ec");
    contract->setStyleSheet("color: #848e9c; font-size: 12px; border: none;");
    mainLayout->addWidget(contract);
}

QWidget* TickerPlaceholder::createStatWidget(const QString &title, const QString &value, const QString &valueColor) {
    QWidget *w = new QWidget();
    w->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *l = new QVBoxLayout(w);
    l->setContentsMargins(0, 10, 0, 10);
    l->setSpacing(2);
    l->setAlignment(Qt::AlignCenter);

    QLabel *lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("color: #848e9c; font-size: 11px; border: none;");
    
    QLabel *lblValue = new QLabel(value);
    QString style = QString("color: %1; font-size: 13px; font-weight: bold; border: none;").arg(valueColor);
    lblValue->setStyleSheet(style);

    l->addWidget(lblTitle);
    l->addWidget(lblValue);
    return w;
}

void TickerPlaceholder::openTickerSelector() {
    TickerSelector *selector = new TickerSelector(this);
    
    // Positionner le popup sous le bouton symbole
    QPoint p = symbolButton->mapToGlobal(QPoint(0, symbolButton->height() + 5));
    selector->move(p);
    
    selector->exec();
    delete selector;
}