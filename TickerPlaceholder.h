#ifndef TICKERPLACEHOLDER_H
#define TICKERPLACEHOLDER_H

#include <QWidget>
#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

// --- Classe pour le menu déroulant (Popup) ---
class TickerSelector : public QDialog {
    Q_OBJECT
public:
    explicit TickerSelector(QWidget *parent = nullptr);

protected:
    // Permet de gérer la perte de focus pour fermer le popup si on clique ailleurs
    bool event(QEvent *event) override;

private:
    QLineEdit *searchEdit;
    QTableWidget *tickerTable;
    void setupUI();
    void loadDummyData(); // Simule les données de l'API
};

// --- Classe pour la barre horizontale principale ---
class TickerPlaceholder : public QWidget {
    Q_OBJECT
public:
    explicit TickerPlaceholder(QWidget *parent = nullptr);

private slots:
    void openTickerSelector();

private:
    // UI Elements
    QPushButton *symbolButton; // Le bouton "HYPE/USDC"

    void setupUI();
    // Helper pour créer les widgets de stats (Price, Volume, etc.)
    QWidget* createStatWidget(const QString &title, const QString &value, const QString &valueColor);
};

#endif // TICKERPLACEHOLDER_H