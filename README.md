# Finance Trading Screen

A trading screen that allows real-time trading simulation, developed in **C++17** and **Qt 6**. This project offers a modern, responsive user interface modeled on industry standards (such as Binance or TradingView), integrating real-time market data feeds and an order execution simulation engine.

---

## 🚀 Main Features & Interconnections

The application is designed to offer a fluid and realistic trading experience. All components are **highly interconnected**:

- **Interactive Chart (ChartWidget)**: Dynamic display of prices in the form of Japanese candlesticks with temporal management and integrated indicators.
- **Order Book (OrderBook)**: Real-time bid/ask visualization of market depth to understand liquidity.
- **Ticker and Market Data (TickerPlaceholder)**: Top banner displaying key 24-hour statistics (Current price, change, absolute volumes).
- **Order Entry & Tracking (OrderEntryPanel & TradingBottomPanel)**: The simulation engine is fully interconnected. **When you place an order** (Market, Limit) via the order entry side panel, this order is instantly processed and routed. The impact is immediately visible in the bottom panel (which tracks history, open orders, and active positions). Everything reacts in real-time, without latency, thanks to Qt's signal/slot system.

---

## 🏗️ Project Architecture

The code was designed according to the principles of separation of concerns (Clean Architecture), clearly distinguishing the application logic (Core) from the user interface (UI).

```text
finance-trading-screen/
├── CMakeLists.txt              # CMake configuration and build script
├── README.md                   # This documentation file
├── build_x64/                  # Directory containing compiled binaries
├── src/                        # Main source code (C++)
│   ├── main.cpp                # Application entry point
│   ├── core/                   # Core logic, data models, and network requests
│   │   └── orderbook.cpp/h     # Order book business logic, JSON parsing, API calls
│   └── ui/                     # Interfaces and graphical components (Qt)
│       ├── MainWindow.cpp/h    # Main window, layout orchestration
│       ├── ChartWidget.cpp/h   # Chart drawing widget (Candlesticks, Volumes, RSI...)
│       ├── OrderEntryPanel.*   # Side panel for placing and adjusting orders
│       ├── TickerPlaceholder.* # Information panel and pair selector
│       └── TradingBottomPanel.*# Bottom panel for portfolio/order tracking
```

---

## 📡 Data Retrieval & API Choice

The initial goal of the academic project planned for an interconnection between the different groups of students (notably with the "Data" group in charge of providing the financial feeds). However, **after multiple synchronization attempts and facing the complexity of agreeing on a functional common interface**, we took the initiative to ensure a 100% operational and autonomous project.

This is why the application currently uses the **public Binance REST API** in real-time. This professional, robust, and well-documented alternative allowed us to develop and validate all the features of our Trading Screen.

However, the project was designed around a **highly modular architecture** in anticipation of the final integration:
- **Network Calls**: The `QtNetwork` module is used to perform asynchronous asynchronous requests in the background so as not to block the interface.
- **API Substitution**: Switching to the Data group's internal API (or any other exchange like Kraken/Bybit) comes down to replacing the base URL (`API_URL`) and ensuring the endpoints match (e.g., `/klines`, `/depth`). As long as the returned JSON format respects the expected structure, the integration effort is minimal.
- **Dynamic Generation**: Requests are built dynamically according to the chosen pair (e.g., `BTCUSDT`, `ETHUSDT`). The JSON parsing, which is very flexible, allows the graphical widgets and the trading engine to remain interoperable and agnostic to the data source.

---

## 🛠️ Launch Instructions

The project was configured with a rigorous CMake file to ensure an "out-of-the-box" compilation.

### System Prerequisites
- **C++17** (MSVC 2022 compiler recommended on MS Windows, GCC/Clang on Linux/Mac)
- **CMake** (version 3.16 minimum)
- **Qt 6.10 or higher** (Make sure you have checked the components: `Core`, `Gui`, `Widgets`, `Charts`, `Sql`, `Network` during installation).

### 🚀 Launch the project step-by-step

1. **Open a command prompt / terminal** in the project's root directory.
2. **Configure the CMake build** (creation of the tree structure and library linking):
   ```bash
   cmake -B build_x64 -S .
   ```
   *(Note: `CMakeLists.txt` will automatically search for `Qt6` on your system via your PATH or the `CMAKE_PREFIX_PATH` variable configured in the file).*

3. **Compile the binaries** (Release mode recommended for optimal performance with temporal charts):
   ```bash
   cmake --build build_x64 --config Release
   ```

4. **Run the application**:
   Once compiled, you can run the application directly. On Windows:
   ```bash
   .\build_x64\Release\TradingLayoutSkeleton.exe
   ```
   *(Or `.\build_x64\TradingLayoutSkeleton.exe` depending on your generator's structure).*

The interface will launch instantly, asynchronously establish its connections to the various APIs to load the default cryptocurrency, and display the markets in real-time!
