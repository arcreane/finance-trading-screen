import sqlite3

try:
    conn = sqlite3.connect('backtest.db')
    cursor = conn.cursor()
    
    print("Tables:")
    cursor.execute("SELECT name FROM sqlite_master WHERE type='table';")
    tables = cursor.fetchall()
    print(tables)
    
    for table in tables:
        table_name = table[0]
        print(f"\nSchema for {table_name}:")
        cursor.execute(f"PRAGMA table_info({table_name})")
        print(cursor.fetchall())
        
        print(f"\nFirst 5 rows of {table_name}:")
        cursor.execute(f"SELECT * FROM {table_name} LIMIT 5")
        print(cursor.fetchall())

except Exception as e:
    print(e)
finally:
    if conn:
        conn.close()
