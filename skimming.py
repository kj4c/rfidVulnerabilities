from flask import Flask, request, jsonify
import sqlite3

app = Flask(__name__)

# db
def get_db_connection():
    conn = sqlite3.connect('rfid_skimming.db')
    conn.row_factory = sqlite3.Row
    return conn

# skimming logs
def init_db():
    conn = get_db_connection()
    conn.execute('''
        CREATE TABLE IF NOT EXISTS skimming_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            rfid_uid TEXT NOT NULL
        )
    ''')
    conn.commit()
    conn.close()

@app.route('/skimming_log', methods=['POST'])
def skimming_log():
    rfid_uid = request.form.get('rfid_uid')
    if not rfid_uid:
        return jsonify({'status': 'fail', 'message': 'rfid_uid is missing'}), 400

    conn = get_db_connection()
    conn.execute('INSERT INTO skimming_logs (rfid_uid) VALUES (?)', (rfid_uid,))
    conn.commit()
    conn.close()

    return jsonify({'status': 'success', 'message': 'Data logged successfully'}), 200

def retrieve_skims():
    conn = get_db_connection()
    logs = conn.execute('SELECT * FROM skimming_logs').fetchall()
    conn.close()
    
    for log in logs:
        print(f"UID: {log['rfid_uid']}")

if __name__ == '__main__':
    init_db()
    retrieve_skims()
    app.run(host='0.0.0.0', port=5000)
