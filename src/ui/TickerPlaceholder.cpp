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

TickerPlaceholder::TickerPlaceholder(QWidget *parent) : QWidget(parent), m_currentSymbol("BTC") {
    setAttribute(Qt::WA_StyledBackground, true);
    setupUI();

    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &TickerPlaceholder::onHttpResponse);

    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(1000); // Rafraîchir toutes les secondes
    connect(m_pollTimer, &QTimer::timeout, this, &TickerPlaceholder::fetchTickerData);
    m_pollTimer->start();

    fetchTickerData();
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

    intervalSelector = new QComboBox(this);
    // Options: "1min", "5min", "1h", "1j"
    intervalSelector->addItem("1min", "1m");
    intervalSelector->addItem("5min", "5m");
    intervalSelector->addItem("1h", "1h");
    intervalSelector->addItem("1j", "1d");
    intervalSelector->setCurrentIndex(2); // Set default to "1h"
    
    intervalSelector->setStyleSheet(
        "QComboBox { background-color: #232832; color: white; border: 1px solid #2a2e39; border-radius: 4px; padding: 2px 10px; font-weight: bold; font-size: 13px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox QAbstractItemView { background-color: #1e222a; color: white; selection-background-color: #2a2e39; }"
    );

    connect(intervalSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        QString interval = intervalSelector->itemData(index).toString();
        emit intervalChanged(interval);
    });

    mainLayout->addWidget(intervalSelector);

    countdownLabel = new QLabel("--:--", this);
    countdownLabel->setStyleSheet("color: #848e9c; font-size: 13px; font-weight: bold; background: #1e222a; padding: 4px 8px; border-radius: 4px; border: 1px solid #2a2e39;");
    mainLayout->addWidget(countdownLabel);
}

QString TickerPlaceholder::currentInterval() const {
    if (intervalSelector) {
        return intervalSelector->currentData().toString();
    }
    return "1h"; // Default
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
    // 1. Mettre à jour le bouton
    symbolButton->setText(data.symbol + "  ▼");

    // 2. Extraire le symbole de base (ex: "BTC/USD" -> "BTC")
    QString cleanSymbol = data.symbol.split("/").first();
    m_currentSymbol = cleanSymbol;

    // 3. Forcer une mise à jour immédiate
    fetchTickerData();

    // 4. Emettre le signal pour le chart et l'orderbook
    emit tickerChanged(cleanSymbol);
}

void TickerPlaceholder::fetchTickerData() {
    // On requête l'API Binance pour le ticker 24h
    QString url = QString("https://api.binance.com/api/v3/ticker/24hr?symbol=%1USDT").arg(m_currentSymbol);
    QNetworkRequest request{QUrl(url)};
    m_networkManager->get(request);

    // Update countdown
    qint64 now = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
    qint64 duration = 0;
    QString intv = currentInterval();
    if (intv == "1m") duration = 60000;
    else if (intv == "5m") duration = 300000;
    else if (intv == "1h") duration = 3600000;
    else if (intv == "1d") duration = 86400000;

    if (duration > 0) {
        qint64 next = ((now / duration) + 1) * duration;
        qint64 diff = next - now; 
        int seconds = (diff / 1000) % 60;
        int minutes = (diff / (1000 * 60)) % 60;
        int hours = (diff / (1000 * 60 * 60)) % 24;
        
        QString text;
        if (intv == "1d") text = QString("%1:%2:%3").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
        else text = QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
        
        countdownLabel->setText(text);
    }
}

void TickerPlaceholder::onHttpResponse(QNetworkReply* reply) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;

    QJsonObject obj = doc.object();

    double lastPrice = obj["lastPrice"].toString().toDouble();
    emit priceUpdated(lastPrice);
    
    double changeVal = obj["priceChange"].toString().toDouble();
    double changePercent = obj["priceChangePercent"].toString().toDouble();
    double quoteVolume = obj["quoteVolume"].toString().toDouble(); // Volume en USDT

    // Formattage du prix
    QString priceStr;
    if (lastPrice >= 1000) {
        priceStr = QString::number(lastPrice, 'f', 2);
        // Ajout d'une virgule pour les milliers
        int pos = priceStr.indexOf('.');
        if (pos > 3) priceStr.insert(pos - 3, ',');
    } else {
        priceStr = QString::number(lastPrice, 'f', 4);
    }

    // Formattage du changement
    QString sign = changeVal > 0 ? "+" : "";
    QString changeStr = QString("%1%2 / %3%4%")
                            .arg(sign).arg(QString::number(changeVal, 'f', 2))
                            .arg(sign).arg(QString::number(changePercent, 'f', 2));

    // Formattage du volume
    QString volStr;
    if (quoteVolume >= 1000000000) {
        volStr = QString::number(quoteVolume / 1000000000, 'f', 2) + "B USD";
    } else if (quoteVolume >= 1000000) {
        volStr = QString::number(quoteVolume / 1000000, 'f', 2) + "M USD";
    } else if (quoteVolume >= 1000) {
        volStr = QString::number(quoteVolume / 1000, 'f', 2) + "K USD";
    } else {
        volStr = QString::number(quoteVolume, 'f', 2) + " USD";
    }

    // Simulation du Market Cap (Binance API 24hr ticker ne donne pas le circulating supply)
    QString capStr;
    if (m_currentSymbol == "BTC") {
        capStr = QString::number((lastPrice * 19600000) / 1000000000, 'f', 2) + "B USD";
    } else if (m_currentSymbol == "ETH") {
        capStr = QString::number((lastPrice * 120000000) / 1000000000, 'f', 2) + "B USD";
    } else {
        // Fallback approximation
        double fakeCap = quoteVolume * 15; 
        if (fakeCap >= 1000000000) capStr = QString::number(fakeCap / 1000000000, 'f', 2) + "B USD";
        else capStr = QString::number(fakeCap / 1000000, 'f', 2) + "M USD";
    }

    // Mises à jour des labels
    priceLabel->setText(priceStr);
    changeLabel->setText(changeStr);
    volumeLabel->setText(volStr);
    capLabel->setText(capStr);

    // Mises à jour des couleurs
    QString color = "#ffffff";
    if (changePercent > 0) {
        color = "#0ecb81";
    } else if (changePercent < 0) {
        color = "#f6465d";
    }

    QString styleTemplate = "color: %1; font-size: 13px; font-weight: bold; border: none;";
    priceLabel->setStyleSheet(styleTemplate.arg(color));
    changeLabel->setStyleSheet(styleTemplate.arg(color));
}