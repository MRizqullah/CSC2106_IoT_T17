from flask import Flask, request

app = Flask(__name__)

@app.route('/api/data', methods=['POST'])
def receive_data():
    data = request.json  # Assuming the data is sent as JSON
    print("Received data:", data)
    return {"status": "success", "message": "Data received"}

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)
