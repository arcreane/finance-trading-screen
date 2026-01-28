#include "TickerPlaceholder.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QEvent>
#include <QBrush>

// ==========================================
// 1. Implémentation du Selecteur (Popup)
// ==========================================

TickerSelector::TickerSelector(QWidget *parent) : QDialog(parent, Qt::Popup | Qt::FramelessWindowHint) {
    setupUI();
    loadDummyData();

    // Connecter le clic sur une cellule à notre slot de traitement
    connect(tickerTable, &QTableWidget::cellClicked, this, &TickerSelector::onRowClicked);
}

void TickerSelector::setupUI() {
    this->setFixedSize(850, 500);
    this->setStyleSheet("background-color: #1b2028; border: 1px solid #2a2e39; border-radius: 8px;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText("Search");
    searchEdit->setStyleSheet(
        "QLineEdit { background-color: #232832; border: 1px solid #2a2e39; padding: 10px; color: white; border-radius: 4px; font-size: 14px; }"
        "QLineEdit:focus { border: 1px solid #4a90e2; }"
    );
    layout->addWidget(searchEdit);

    tickerTable = new QTableWidget(this);
    tickerTable->setColumnCount(6);
    tickerTable->setHorizontalHeaderLabels({"Symbol", "Last Price", "24h Change", "8hr Funding", "Volume", "Open Interest"});

    tickerTable->verticalHeader()->setVisible(false);
    tickerTable->setShowGrid(false);
    tickerTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tickerTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tickerTable->setFocusPolicy(Qt::NoFocus);

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
    // Données simulées - Only BTC and ETH
    struct CoinEntry { QString sym; QString price; QString chg; QString fund; QString vol; QString oi; };
    QList<CoinEntry> data = {
        {"BTC/USD", "48500.00", "+1200 / +2.54%", "0.0100%", "1.2B USD", "5.5B"},
        {"ETH/USD", "2850.00", "+45.00 / +1.60%", "0.0100%", "650M USD", "2.1B"}
    };

    tickerTable->setRowCount(data.size());

    for (int i = 0; i < data.size(); ++i) {
        // Ajout de l'étoile visuelle mais on ne la stockera pas dans les données brutes
        QTableWidgetItem *sym = new QTableWidgetItem("★  " + data[i].sym);
        sym->setData(Qt::UserRole, data[i].sym); // On stocke le vrai symbole proprement
        sym->setForeground(QBrush(QColor("white")));
        tickerTable->setItem(i, 0, sym);

        QTableWidgetItem *price = new QTableWidgetItem(data[i].price);
        price->setForeground(QBrush(QColor("white")));
        tickerTable->setItem(i, 1, price);

        QTableWidgetItem *chg = new QTableWidgetItem(data[i].chg);
        if (data[i].chg.contains("+")) chg->setForeground(QBrush(QColor("#0ecb81")));
        else chg->setForeground(QBrush(QColor("#f6465d")));
        tickerTable->setItem(i, 2, chg);

        tickerTable->setItem(i, 3, new QTableWidgetItem(data[i].fund));
        tickerTable->setItem(i, 4, new QTableWidgetItem(data[i].vol));
        tickerTable->setItem(i, 5, new QTableWidgetItem(data[i].oi));
    }
}

void TickerSelector::onRowClicked(int row, int col) {
    Q_UNUSED(col);
    TickerData data;

    // Récupération des données depuis le tableau
    // Note: column 0 contient "★  SYMBOL", on récupère la UserData ou on nettoie le texte
    data.symbol = tickerTable->item(row, 0)->data(Qt::UserRole).toString();
    if(data.symbol.isEmpty()) data.symbol = tickerTable->item(row, 0)->text().remove("★  ");

    data.price = tickerTable->item(row, 1)->text();
    data.change = tickerTable->item(row, 2)->text();
    // Le volume est à l'index 4
    data.volume = tickerTable->item(row, 4)->text();
    // On simule un Market Cap basé sur le volume pour l'exemple
    data.marketCap = "12B USDC";

    emit tickerSelected(data); // Envoyer le signal
    this->accept(); // Fermer la fenêtre
}

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
    setAttribute(Qt::WA_StyledBackground, true);
    setupUI();
}

void TickerPlaceholder::setupUI() {
    this->setFixedHeight(45);
    this->setStyleSheet("background-color: #161616; border-bottom: 1px solid #2a2e39;");

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 0, 15, 0);
    mainLayout->setSpacing(20);

    // --- Section Gauche ---
    QWidget *leftContainer = new QWidget();
    leftContainer->setStyleSheet("background: transparent; border: none;");
    QHBoxLayout *leftLayout = new QHBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0,0,0,0);
    leftLayout->setSpacing(8);

    QLabel *logo = new QLabel();
    logo->setFixedSize(20, 20);
    logo->setStyleSheet("background-color: #7df2d5; border-radius: 10px; border: none;");

    symbolButton = new QPushButton("BTC/USD  ▼");
    symbolButton->setCursor(Qt::PointingHandCursor);
    symbolButton->setStyleSheet(
        "QPushButton {"
        "    color: white;"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    background-color: #232832;"
        "    border: 1px solid #2a2e39;"
        "    border-radius: 6px;"
        "    padding: 6px 12px;"
        "    text-align: left;"
        "}"
        "QPushButton:hover {"
        "    background-color: #2a2e39;"
        "    border-color: #3a3e49;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1e222a;"
        "}"
    );
    connect(symbolButton, &QPushButton::clicked, this, &TickerPlaceholder::openTickerSelector);

    QLabel *badge = new QLabel("Spot");
    badge->setStyleSheet("background-color: #1e3a3a; color: #3BB3A3; padding: 2px 6px; border-radius: 4px; font-weight: bold; font-size: 11px; border: none;");

    leftLayout->addWidget(logo);
    leftLayout->addWidget(symbolButton);
    leftLayout->addWidget(badge);

    mainLayout->addWidget(leftContainer);

    QFrame *vLine = new QFrame;
    vLine->setFrameShape(QFrame::VLine);
    vLine->setStyleSheet("color: #2a2e39;");
    mainLayout->addWidget(vLine);

    // --- Section Stats (Avec passage de référence pour les labels) ---
    // Price
    mainLayout->addWidget(createStatWidget("Price", "48,500.00", "#0ecb81", &priceLabel));
    // Change
    mainLayout->addWidget(createStatWidget("24h Change", "+1,200 / +2.54%", "#0ecb81", &changeLabel));
    // Volume
    mainLayout->addWidget(createStatWidget("24h Volume", "1.2B USD", "white", &volumeLabel));
    // Cap
    mainLayout->addWidget(createStatWidget("Market Cap", "950B USD", "white", &capLabel));

    mainLayout->addStretch();

    QLabel *contract = new QLabel("Contract 0x0d01...11ec");
    contract->setStyleSheet("color: #848e9c; font-size: 12px; border: none;");
    mainLayout->addWidget(contract);
}

// Fonction helper modifiée pour stocker le pointeur du label créé
QWidget* TickerPlaceholder::createStatWidget(const QString &title, const QString &initialValue, const QString &color, QLabel **memberLabelPtr) {
    QWidget *w = new QWidget();
    w->setStyleSheet("background: transparent; border: none;");
    QVBoxLayout *l = new QVBoxLayout(w);
    l->setContentsMargins(0, 5, 0, 5);
    l->setSpacing(2);
    l->setAlignment(Qt::AlignCenter);

    QLabel *lblTitle = new QLabel(title);
    lblTitle->setStyleSheet("color: #848e9c; font-size: 11px; border: none;");

    QLabel *lblValue = new QLabel(initialValue);
    QString style = QString("color: %1; font-size: 13px; font-weight: bold; border: none;").arg(color);
    lblValue->setStyleSheet(style);

    // IMPORTANT : On sauvegarde le pointeur du label dans notre variable membre
    *memberLabelPtr = lblValue;

    l->addWidget(lblTitle);
    l->addWidget(lblValue);
    return w;
}

void TickerPlaceholder::openTickerSelector() {
    // Éviter la réouverture immédiate après fermeture (le popup se ferme quand on clique sur le bouton)
    // Si le popup a été fermé il y a moins de 200ms, on ignore le clic
    if (selectorCloseTimer.isValid() && selectorCloseTimer.elapsed() < 200) {
        return;
    }

    // Toggle: si le sélecteur est déjà ouvert, on le ferme
    if (tickerSelector != nullptr && tickerSelector->isVisible()) {
        tickerSelector->close();
        return;
    }

    // Créer un nouveau sélecteur
    tickerSelector = new TickerSelector(this);

    // Positionner le popup
    QPoint p = symbolButton->mapToGlobal(QPoint(0, symbolButton->height() + 5));
    tickerSelector->move(p);

    // CONNECTER LE SIGNAL DU POPUP AU SLOT DE LA BARRE
    connect(tickerSelector, &TickerSelector::tickerSelected, this, &TickerPlaceholder::updateTickerDisplay);

    tickerSelector->exec();
    
    // Démarrer le timer quand le popup se ferme
    selectorCloseTimer.start();
    
    delete tickerSelector;
    tickerSelector = nullptr;
}

void TickerPlaceholder::updateTickerDisplay(const TickerData &data) {
    // 1. Mettre à jour les textes
    symbolButton->setText(data.symbol + "  ▼");
    priceLabel->setText(data.price);
    changeLabel->setText(data.change);
    volumeLabel->setText(data.volume);
    capLabel->setText(data.marketCap);

    // 2. Mettre à jour les couleurs dynamiquement (Vert si positif, Rouge si négatif)
    QString color = "#ffffff"; // Default white
    if (data.change.contains("+")) {
        color = "#0ecb81"; // Green
    } else if (data.change.contains("-")) {
        color = "#f6465d"; // Red
    }

    // Appliquer le style au Prix et au Change
    QString styleTemplate = "color: %1; font-size: 13px; font-weight: bold; border: none;";
    priceLabel->setStyleSheet(styleTemplate.arg(color));
    changeLabel->setStyleSheet(styleTemplate.arg(color));

    // 3. Emit signal for chart update - extract base symbol (e.g., "BTC/USD" -> "BTC")
    QString cleanSymbol = data.symbol.split("/").first();
    emit tickerChanged(cleanSymbol);
}