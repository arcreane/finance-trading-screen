#ifndef TICKERPLACEHOLDER_H
#define TICKERPLACEHOLDER_H

#include <QWidget>
#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QElapsedTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QComboBox>
#include <QDateTime>

// Structure pour transporter les données d'une crypto
struct TickerData {
    QString symbol;
    QString price;
    QString change; // ex: "-5.16 / -3.64%"
    QString volume;
    QString marketCap;
};

// --- Classe pour le menu déroulant (Popup) ---
class TickerSelector : public QDialog {
    Q_OBJECT
public:
    explicit TickerSelector(QWidget *parent = nullptr);

    signals:
        // Signal émis quand une ligne est choisie
        void tickerSelected(const TickerData &data);

protected:
    bool event(QEvent *event) override;

private slots:
    // Slot interne quand on clique sur le tableau
    void onRowClicked(int row, int col);

private:
    QLineEdit *searchEdit;
    QTableWidget *tickerTable;
    void setupUI();
    void loadDummyData();
};

// --- Classe pour la barre principale ---
class TickerPlaceholder : public QWidget {
    Q_OBJECT
public:
    explicit TickerPlaceholder(QWidget *parent = nullptr);

signals:
    // Signal emitted when user selects a different coin
    void tickerChanged(const QString &symbol);
    void priceUpdated(double price);
    void intervalChanged(const QString &interval);

public:
    QString currentSymbol() const { return m_currentSymbol; }
    QString currentInterval() const;

private slots:
    void openTickerSelector();
    // Slot pour recevoir les données et mettre à jour l'UI
    void updateTickerDisplay(const TickerData &data);
    void fetchTickerData();
    void onHttpResponse(QNetworkReply* reply);

private:
    // UI Elements interactifs (stockés pour pouvoir les modifier)
    QPushButton *symbolButton;
    TickerSelector *tickerSelector = nullptr; // Pointeur vers le popup pour toggle
    QElapsedTimer selectorCloseTimer; // Timer pour éviter réouverture immédiate
    QComboBox *intervalSelector;
    QLabel *countdownLabel;
    QLabel *priceLabel;
    QLabel *changeLabel;
    QLabel *volumeLabel;
    QLabel *capLabel;

    QNetworkAccessManager *m_networkManager;
    QTimer *m_pollTimer;
    QString m_currentSymbol;

    void setupUI();

    // Modifié pour assigner le pointeur du label créé à notre variable membre
    QWidget* createStatWidget(const QString &title, const QString &initialValue, const QString &color, QLabel **memberLabelPtr);
};

#endif // TICKERPLACEHOLDER_H