import sqlite3
import urllib.request
import json
import sys
import time
from datetime import datetime

# Configuration
API_KEY = "YOUR_API_KEY_HERE" # REPLACE THIS WITH YOUR ALPHA VANTAGE API KEY
SYMBOL = "BTC"
MARKET = "USD"
DB_NAME = "backtest.db"

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

def fetch_data():
    url = f"https://www.alphavantage.co/query?function=DIGITAL_CURRENCY_DAILY&symbol={SYMBOL}&market={MARKET}&apikey={API_KEY}"
    print(f"Fetching data from: {url.replace(API_KEY, 'HIDDEN_KEY')}")
    
    try:
        with urllib.request.urlopen(url) as response:
            data = json.loads(response.read().decode())
            
        if "Error Message" in data:
            print(f"Error from API: {data['Error Message']}")
            return None
        if "Note" in data:
            print(f"API Note: {data['Note']}")
            
        return data
    except Exception as e:
        print(f"Error fetching data: {e}")
        return None

def populate_db(data):
    if not data or "Time Series (Digital Currency Daily)" not in data:
        print("No valid data found in response.")
        return

    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    create_table(cursor)

    time_series = data["Time Series (Digital Currency Daily)"]
    count = 0
    
    print(f"Processing {len(time_series)} records...")
    
    for date_str, values in time_series.items():
        # Format: YYYY-MM-DD
        # We need to append time for the C++ app: YYYY-MM-DD HH:mm:ss
        timestamp = f"{date_str} 00:00:00"
        
        open_price = float(values["1a. open (USD)"])
        high_price = float(values["2a. high (USD)"])
        low_price = float(values["3a. low (USD)"])
        close_price = float(values["4a. close (USD)"])
        volume = float(values["5. volume"])
        
        try:
            cursor.execute('''
                INSERT OR REPLACE INTO StockData (Symbol, Timestamp, Open, High, Low, Close, Volume)
                VALUES (?, ?, ?, ?, ?, ?, ?)
            ''', (SYMBOL, timestamp, open_price, high_price, low_price, close_price, volume))
            count += 1
        except sqlite3.Error as e:
            print(f"Database error: {e}")

    conn.commit()
    conn.close()
    print(f"Successfully inserted/updated {count} records in {DB_NAME}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        API_KEY = sys.argv[1]
    
    if API_KEY == "YOUR_API_KEY_HERE":
        print("Please provide an API key as an argument or edit the script.")
        print("Usage: python fetch_data.py <API_KEY>")
        sys.exit(1)

    data = fetch_data()
    populate_db(data)
