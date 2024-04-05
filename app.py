from flask import Flask, render_template, request
from flask_socketio import SocketIO, emit
import random

app = Flask(__name__)
app.config['SECRET_KEY'] = 'your-secret-key'
socketio = SocketIO(app)

indoor_devices = set()
outdoor_devices = set()
ble_devices = set()

@app.route('/')
def dashboard():
    return render_template('dashboard.html')

@socketio.on('connect')
def handle_connect():
    emit('device_update', {'indoor': len(indoor_devices), 'outdoor': len(outdoor_devices)})

@socketio.on('device_signal')
def handle_device_signal(data):
    device_id = data['device_id']
    indoor_signal = data['indoor_signal']
    outdoor_signal = data['outdoor_signal']

    if indoor_signal > outdoor_signal:
        indoor_devices.add(device_id)
        outdoor_devices.discard(device_id)
    else:
        outdoor_devices.add(device_id)
        indoor_devices.discard(device_id)

    emit('device_update', {'indoor': len(indoor_devices), 'outdoor': len(outdoor_devices)}, broadcast=True)
    emit('signal_update', {'indoor_signal': indoor_signal, 'outdoor_signal': outdoor_signal}, broadcast=True)

@app.route('/device_signal', methods=['POST'])
def handle_device_signal_post():
    data = request.get_json()
    device_id = data['device_id']
    indoor_signal = data['indoor_signal']
    outdoor_signal = data['outdoor_signal']

    if indoor_signal > outdoor_signal:
        indoor_devices.add(device_id)
        outdoor_devices.discard(device_id)
    else:
        outdoor_devices.add(device_id)
        indoor_devices.discard(device_id)

    socketio.emit('device_update', {'indoor': len(indoor_devices), 'outdoor': len(outdoor_devices)}, broadcast=True)
    socketio.emit('signal_update', {'indoor_signal': indoor_signal, 'outdoor_signal': outdoor_signal}, broadcast=True)
    return 'OK'

@app.route('/api/data', methods=['POST'])
def receive_data():
    data = request.get_json()
    print("Received data:", data)

    device_mac = data['mac']
    rssi = data['rssi']

    if rssi >= -30:
        indoor_devices.add(device_mac)
        outdoor_devices.discard(device_mac)
    else:
        outdoor_devices.add(device_mac)
        indoor_devices.discard(device_mac)

    socketio.emit('device_update', {'indoor': len(indoor_devices), 'outdoor': len(outdoor_devices)}, broadcast=True)
    socketio.emit('signal_update', {'rssi': rssi}, broadcast=True)

    return jsonify({"status": "success", "message": "Data received"})

@app.route('/api/ble_data', methods=['POST'])
def receive_ble_data():
    data = request.get_json()
    print("Received BLE data:", data)

    for device in data:
        device_mac = device['mac_address']
        rssi = device['rssi']
        board_name = device['boardName']

        if rssi >= -30:
            ble_devices.add(device_mac)
        else:
            ble_devices.discard(device_mac)

        socketio.emit('ble_device_update', {'ble_devices': len(ble_devices)}, broadcast=True)
        socketio.emit('ble_signal_update', {'mac_address': device_mac, 'rssi': rssi, 'board_name': board_name}, broadcast=True)

    return jsonify({"status": "success", "message": "BLE data received"})

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)