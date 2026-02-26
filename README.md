# Finance Trading Screen

Un Trading screen qui permet la simulation de trading en temps réel, développée en **C++17** et **Qt 6**. Ce projet offre une interface utilisateur moderne, réactive et calquée sur les standards de l'industrie (comme Binance ou TradingView), en intégrant des flux de données de marché en temps réel et un moteur de simulation de passage d'ordres.

---

## 🚀 Fonctionnalités Principales & Interconnexions

L'application est conçue pour offrir une expérience de trading fluide et réaliste. Tous les composants sont **fortement interconnectés** :

- **Graphique Interactif (ChartWidget)** : Affichage dynamique des cours sous forme de chandeliers japonais (Candlesticks) avec gestion temporelle et indicateurs intégrés.
- **Carnet d'Ordres (OrderBook)** : Visualisation bid/ask de la profondeur du marché en temps réel pour comprendre la liquidité.
- **Ticker et Données de Marché (TickerPlaceholder)** : Bandeau supérieur affichant les statistiques clés sur 24 heures (Prix actuel, variation, volumes absolus).
- **Passage d'Ordres & Suivi (OrderEntryPanel & TradingBottomPanel)** : Le moteur de simulation est pleinement interconnecté. **Lorsque vous placez un ordre** (Market, Limit) via le panneau latéral de passage d'ordres, cet ordre est instantanément traité et routé. L'impact est immédiatement visible dans le panneau inférieur (qui trace l'historique, les ordres ouverts et les positions actives). Tout réagit en temps réel, sans latence, grâce au système de signaux/slots de Qt.

---

## 🏗️ Architecture du Projet

Le code a été pensé selon des principes de séparation des préoccupations (Clean Architecture), distinguant nettement la logique applicative (Core) de l'interface utilisateur (UI).

```text
finance-trading-screen/
├── CMakeLists.txt              # Script de configuration et de build CMake
├── README.md                   # Ce fichier de documentation
├── build_x64/                  # Répertoire contenant les binaires compilés
├── src/                        # Code source principal (C++)
│   ├── main.cpp                # Point d'entrée de l'application
│   ├── core/                   # Cœur logique, modèles de données et requêtes réseau
│   │   └── orderbook.cpp/h     # Logique métier du carnet d'ordres, parsing JSON, appels API
│   └── ui/                     # Interfaces et composants graphiques (Qt)
│       ├── MainWindow.cpp/h    # Fenêtre principale, orchestration de la disposition
│       ├── ChartWidget.cpp/h   # Widget de dessin du graphique (Chandeliers, Volumes, RSI...)
│       ├── OrderEntryPanel.*   # Panneau latéral de passage et de réglage des ordres
│       ├── TickerPlaceholder.* # Panneau d'informations et sélecteur de paires
│       └── TradingBottomPanel.*# Panneau inférieur de suivi de portefeuilles/ordres
```

---

## 📡 Récupération des Données & Choix de l'API

L'objectif initial du projet académique prévoyait une interconnexion entre les différents groupes d'étudiants (notamment avec le groupe "Data" en charge de fournir les flux financiers). Cependant, **après de multiples tentatives de synchronisation et face à la complexité de se mettre d'accord sur une interface commune fonctionnelle**, nous avons pris l'initiative d'assurer un projet 100% opérationnel et autonome. 

C'est pourquoi l'application utilise actuellement **l'API REST publique de Binance** en temps réel. Cette alternative professionnelle, robuste et bien documentée nous a permis de développer et de valider toutes les fonctionnalités de notre Trading Screen.

Cependant, le projet a été pensé autour d'une **architecture hautement modulaire** en prévision de l'intégration finale :
- **Appels Réseau** : Le module `QtNetwork` est utilisé pour effectuer des requêtes asynchrones en arrière-plan afin de ne pas bloquer l'interface.
- **Substitution d'API** : Le basculement vers l'API interne du groupe Data (ou tout autre exchange comme Kraken/Bybit) se résume à remplacer l'URL de base (`API_URL`) et à s'assurer de la correspondance des endpoints (ex: `/klines`, `/depth`). Tant que le format JSON retourné respecte la structure attendue, l'effort d'intégration est minime.
- **Génération Dynamique** : Les requêtes sont construites dynamiquement selon la paire choisie (ex: `BTCUSDT`, `ETHUSDT`). Le parsing JSON, très flexible, permet aux widgets graphiques et au moteur de trading de rester interopérables et agnostiques par rapport à la source de données.

---

## 🛠️ Instructions de Lancement 

Le projet a été configuré avec un fichier CMake rigoureux pour assurer une compilation "out-of-the-box".

### Prérequis Systèmes
- **C++17** (Compilateur MSVC 2022 recommandé sur MS Windows, GCC/Clang sur Linux/Mac)
- **CMake** (version 3.16 minimum)
- **Qt 6.10 ou supérieur** (Assurez-vous d'avoir coché les composants : `Core`, `Gui`, `Widgets`, `Charts`, `Sql`, `Network` lors de l'installation).

### 🚀 Lancer le projet pas-à-pas

1. **Ouvrir une invite de commande / terminal** dans le répertoire racine du projet.
2. **Configurer le build CMake** (création de l'arborescence et liaison des bibliothèques) :
   ```bash
   cmake -B build_x64 -S .
   ```
   *(Note : le `CMakeLists.txt` recherchera automatiquement `Qt6` sur votre système via votre PATH ou la variable `CMAKE_PREFIX_PATH` configurée dans le fichier).*

3. **Compiler les binaires** (mode Release recommandé pour des performances optimales avec les graphiques temporels) :
   ```bash
   cmake --build build_x64 --config Release
   ```

4. **Exécuter l'application** :
   Une fois compilé, vous pouvez lancer l'application directement. Sous Windows :
   ```bash
   .\build_x64\Release\TradingLayoutSkeleton.exe
   ```
   *(Ou `.\build_x64\TradingLayoutSkeleton.exe` selon la structure de votre générateur).*

L'interface se lancera instantanément, établira de façon asynchrone ses connexions aux différentes API pour charger la crypto-monnaie par défaut, et affichera les marchés en temps réel !
