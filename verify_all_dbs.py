import sqlite3
import os

db_paths = [
    'build/data/backtest.db',
    'build/backtest.db',
    'data/backtest.db',
    'backtest.db'
]

for db_path in db_paths:
    print(f"\nChecking {db_path}...")
    if not os.path.exists(db_path):
        print("  File not found.")
        continue
        
    try:
        size = os.path.getsize(db_path)
        print(f"  Size: {size}")
        if size == 0:
            print("  Empty file.")
            continue
            
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        cursor.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='StockData'")
        if not cursor.fetchone():
             print("  Table StockData not found.")
        else:
            cursor.execute('SELECT COUNT(*) FROM StockData')
            count = cursor.fetchone()[0]
            print(f"  Total rows in StockData: {count}")
            cursor.execute('SELECT MAX(Timestamp) FROM StockData')
            print(f"  Max Timestamp: {cursor.fetchone()[0]}")
            
        conn.close()
    except Exception as e:
        print(f"  Error: {e}")
