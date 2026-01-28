import sqlite3
import os

db_path = 'data/backtest.db'
if not os.path.exists(db_path):
    print(f"File {db_path} does not exist!")
    exit(1)

print(f"Checking {db_path}...")
try:
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()
    cursor.execute('SELECT COUNT(*) FROM StockData')
    count = cursor.fetchone()[0]
    print(f"Total rows in StockData: {count}")
    
    cursor.execute('SELECT * FROM StockData LIMIT 5')
    rows = cursor.fetchall()
    for row in rows:
        print(row)
    conn.close()
except Exception as e:
    print(f"Error: {e}")
