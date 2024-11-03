from flask import Flask, request, jsonify
import sqlite3

app = Flask(__name__)

def get_db_connection():
    conn = sqlite3.connect('rfid_users.db')
    conn.row_factory = sqlite3.Row
    return conn

# creates the db and inserts the admin user
def init_db():
    conn = get_db_connection()
    conn.execute('''
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL
        )
    ''')
    # admin card
    conn.execute("INSERT INTO users (username) VALUES ('83861bad')")
    conn.commit()
    conn.close()

@app.route('/handle_rfid', methods=['POST'])
def handle_rfid():
    username = request.form.get('username')
    
    if not username:
        return jsonify({'status': 'fail', 'message': 'username missing'}), 400

    conn = get_db_connection()

    query = f"SELECT * FROM users WHERE username = '{username}'"
    print(f"Executing Query: {query}")
    
    try:
        user = conn.execute(query).fetchone()
        conn.close()

        if user:
            return jsonify({'status': 'success', 'message': 'you logged in!'}), 200
        else:
            return jsonify({'status': 'fail', 'message': 'username is not in the database'}), 404

    except Exception as e:
        return jsonify({'status': 'error', 'message': str(e)}), 500


if __name__ == '__main__':
    init_db()
    app.run(host='0.0.0.0', port=5000)