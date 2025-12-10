import sqlite3
import random
from datetime import datetime, timedelta

DB_NAME = "backtest.db"
SYMBOL = "BTC"

def create_table(cursor):
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS StockData (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            Symbol TEXT NOT NULL,
            Timestamp TEXT NOT NULL,
            Open REAL,
            High REAL,
            Low REAL,
            Close REAL,
            Volume REAL,
            UNIQUE(Symbol, Timestamp)
        )
    ''')

def generate_data():
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    create_table(cursor)

    # Start date: 1 year ago
    current_date = datetime.now() - timedelta(days=365)
    end_date = datetime.now()
    
    # Initial price
    price = 40000.0
    
    print(f"Generating dummy data for {SYMBOL}...")
    count = 0
    
    while current_date <= end_date:
        timestamp = current_date.strftime("%Y-%m-%d 00:00:00")
        
        # Random daily movement (-5% to +5%)
        change_pct = random.uniform(-0.05, 0.05)
        
        open_price = price
        close_price = price * (1 + change_pct)
        
        # High/Low relative to Open/Close
        high_price = max(open_price, close_price) * (1 + random.uniform(0, 0.02))
        low_price = min(open_price, close_price) * (1 - random.uniform(0, 0.02))
        
        # Random Volume
        volume = random.uniform(1000, 5000)

        try:
            cursor.execute('''
                INSERT OR REPLACE INTO StockData (Symbol, Timestamp, Open, High, Low, Close, Volume)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            ''', (SYMBOL, timestamp, open_price, high_price, low_price, close_price, volume))
            count += 1
        except sqlite3.Error as e:
            print(f"Database error: {e}")
            
        # Update price for next day
        price = close_price
        current_date += timedelta(days=1)

    conn.commit()
    conn.close()
    print(f"Successfully generated {count} records in {DB_NAME}")

if __name__ == "__main__":
    generate_data()
