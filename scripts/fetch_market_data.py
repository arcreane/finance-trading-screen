#!/usr/bin/env python3
"""
Fetches historical Bitcoin (BTC) and Ethereum (ETH) candlestick data
from Binance API and inserts it into backtest.db.
"""
import sqlite3
import requests
import shutil
import os
from datetime import datetime

BINANCE_API_URL = "https://api.binance.com/api/v3/klines"

# Symbols to fetch (Binance symbol -> our symbol)
SYMBOLS = {
    "BTCUSDT": "BTC",
    "ETHUSDT": "ETH",
}

# 1 day interval, 365 days of data
INTERVAL = "1d"
LIMIT = 365


def fetch_klines(symbol: str, interval: str, limit: int) -> list:
    """Fetch candlestick data from Binance API."""
    params = {
        "symbol": symbol,
        "interval": interval,
        "limit": limit,
    }
    response = requests.get(BINANCE_API_URL, params=params, timeout=30)
    response.raise_for_status()
    return response.json()


def main():
    # Determine paths relative to script location
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    
    # Target: project_root/data/backtest.db
    data_dir = os.path.join(project_root, "data")
    if not os.path.exists(data_dir):
        os.makedirs(data_dir)
        
    db_path = os.path.join(data_dir, "backtest.db")
    print(f"Database path: {db_path}")

    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    # Ensure table exists
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS StockData (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            Symbol TEXT NOT NULL,
            Timestamp TEXT NOT NULL,
            Open REAL,
            High REAL,
            Low REAL,
            Close REAL,
            Volume REAL
        )
    """)

    for binance_symbol, our_symbol in SYMBOLS.items():
        print(f"Fetching {binance_symbol}...")

        # Clear existing data for this symbol
        cursor.execute("DELETE FROM StockData WHERE Symbol = ?", (our_symbol,))

        # Fetch data from Binance
        klines = fetch_klines(binance_symbol, INTERVAL, LIMIT)

        # Insert data
        for kline in klines:
            # Kline format: [open_time, open, high, low, close, volume, ...]
            open_time_ms = kline[0]
            open_price = float(kline[1])
            high_price = float(kline[2])
            low_price = float(kline[3])
            close_price = float(kline[4])
            volume = float(kline[5])

            # Convert timestamp
            timestamp_str = datetime.utcfromtimestamp(open_time_ms / 1000).strftime(
                "%Y-%m-%d %H:%M:%S"
            )

            cursor.execute(
                """
                INSERT INTO StockData (Symbol, Timestamp, Open, High, Low, Close, Volume)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            """,
                (our_symbol, timestamp_str, open_price, high_price, low_price, close_price, volume),
            )

        print(f"  Inserted {len(klines)} records for {our_symbol}")

    conn.commit()

    # Verify
    cursor.execute("SELECT Symbol, COUNT(*) FROM StockData GROUP BY Symbol")
    counts = cursor.fetchall()
    print("\nDatabase summary:")
    for symbol, count in counts:
        print(f"  {symbol}: {count} records")

    conn.close()

    # Sync database to build directory if it exists
    # Target: project_root/build/data/backtest.db
    build_dir = os.path.join(project_root, "build")
    
    if os.path.isdir(build_dir):
        build_data_dir = os.path.join(build_dir, "data")
        if not os.path.exists(build_data_dir):
            os.makedirs(build_data_dir)
            
        build_db = os.path.join(build_data_dir, "backtest.db")
        shutil.copy2(db_path, build_db)
        print(f"Synced database to {build_db}")

    print("\nDone!")


if __name__ == "__main__":
    main()
