# Finance Trading Screen

Application de simulation d'écran de trading financier développée avec **Qt6** et **C++**.

---

## Fonctionnalités

- **Graphique de trading** : Visualisation des cours en temps réel avec chandeliers japonais (candlestick chart).
- **Order Book** : Affichage du carnet d'ordres avec les niveaux d'achat (Bid) et de vente (Ask).
- **Panneau d'entrée d'ordres** : Interface pour passer  des ordres (Market, Limit, etc.).
- **Ticker** : Affichage des informations clés sur l'actif sélectionné.
- **Panneau inférieur** : Historique des ordres et positions.

---

## Prérequis

- **Qt 6.10+** (avec les composants : Core, Gui, Widgets, Charts, Sql)
- **CMake 3.16+**
- **Compilateur C++17** (MSVC 2022 recommandé sur Windows)

---

## Lancement du projet

### 1. Configurer le projet avec CMake

```bash
mkdir build
cd build
cmake ..
```

> **Note** : Assurez-vous que le chemin vers Qt est correctement défini dans `CMakeLists.txt` (voir section ci-dessous).

### 2. Compiler le projet

```bash
cmake --build .
```

### 3. Exécuter l'application

```bash
./TradingLayoutSkeleton
```

Sur Windows :
```bash
.\TradingLayoutSkeleton.exe
```

---

## Fichier CMakeLists.txt

Voici le contenu du fichier `CMakeLists.txt` utilisé pour configurer le projet :

```cmake
cmake_minimum_required(VERSION 3.16)

project(TradingLayoutSkeleton LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set Qt path BEFORE find_package
set(CMAKE_PREFIX_PATH "C:/Qt/6.10.0/msvc2022_64")

# Find Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts Sql)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.h
        TickerPlaceholder.cpp
        TickerPlaceholder.h
        ChartWidget.cpp
        ChartWidget.h
        orderbook.cpp
        orderbook.h
        TradingBottomPanel.cpp
        TradingBottomPanel.h
        OrderEntryPanel.cpp
        OrderEntryPanel.h
)

add_executable(TradingLayoutSkeleton ${PROJECT_SOURCES})

target_link_libraries(TradingLayoutSkeleton PRIVATE Qt6::Widgets Qt6::Charts Qt6::Sql)
```

> **Important** : Modifiez la ligne `CMAKE_PREFIX_PATH` pour correspondre à votre installation de Qt si nécessaire.

---

## Données locales

⚠️ **N'oubliez pas les fichiers de données locaux !**

L'application utilise des fichiers de données locaux pour fonctionner :

- `backtest.db` : Base de données SQLite contenant les données historiques.
- `orderbook_BTC_*.json` : Fichiers JSON contenant les données du carnet d'ordres.

Ces fichiers doivent être présents à la racine du projet ou dans le répertoire d'exécution.

---

## Structure du projet

```
finance-trading-screen/
├── CMakeLists.txt              # Configuration CMake
├── README.md                   # Documentation
├── src/                        # Code source C++
│   ├── main.cpp                # Point d'entrée
│   ├── ui/                     # Widgets interface utilisateur
│   │   ├── MainWindow.cpp/h    # Fenêtre principale
│   │   ├── ChartWidget.cpp/h   # Widget graphique candlestick + RSI
│   │   ├── OrderEntryPanel.cpp/h # Panneau d'entrée d'ordres
│   │   ├── TickerPlaceholder.cpp/h # Sélecteur de ticker
│   │   └── TradingBottomPanel.cpp/h # Panneau inférieur
│   └── core/                   # Logique métier
│       └── orderbook.cpp/h     # Gestion du carnet d'ordres
├── data/                       # Fichiers de données
│   ├── backtest.db             # Base SQLite (données historiques)
│   └── orderbook_BTC_*.json    # Données order book
├── scripts/                    # Scripts Python utilitaires
│   ├── fetch_market_data.py    # Récupération données marché
│   └── insert_btc_data.py      # Insertion données BTC
├── nlohmann/                   # Bibliothèque JSON header-only
└── build/                      # Dossier de build (généré)
```

