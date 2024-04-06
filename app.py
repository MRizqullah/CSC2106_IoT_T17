import eventlet
eventlet.monkey_patch()

from flask import Flask, render_template, request, jsonify
from flask_socketio import SocketIO
from pydash import throttle

app = Flask(__name__)
app.config['SECRET_KEY'] = 'your-secret-key'
socketio = SocketIO(app, async_mode='eventlet')

watch_nodes = {}
node_devices = {}

@app.route('/')
def dashboard():
    return render_template('dashboard.html', watch_nodes=watch_nodes, node_devices=node_devices)

def emit_tag_update(mac_address, node_type, rssi, node_mac):
    socketio.emit('tag_update', {'mac_address': mac_address, 'node_type': node_type, 'rssi': rssi, 'node_mac': node_mac}, namespace='/')

def emit_device_count_update(node_mac, count):
    socketio.emit('device_count_update', {'node_mac': node_mac, 'count': count}, namespace='/')

# Increased throttle duration to 2 seconds
throttled_tag_update = throttle(emit_tag_update, 2)
throttled_device_count_update = throttle(emit_device_count_update, 2)

@app.route('/api/data', methods=['POST'])
def receive_data():
    try:
        data = request.get_json()
        mac_address = data['mac']
        rssi = data['rssi']
        node_mac = data['node_mac']
        
        if mac_address not in watch_nodes or watch_nodes[mac_address]['type'] != 'BLE':
            if mac_address not in watch_nodes or rssi > watch_nodes[mac_address]['rssi']:
                watch_nodes[mac_address] = {'type': 'LoRa', 'rssi': rssi, 'node_mac': node_mac}
                throttled_tag_update(mac_address, 'LoRa', rssi, node_mac)

        if mac_address in watch_nodes and watch_nodes[mac_address]['type'] == 'LoRa':
            old_node_mac = watch_nodes[mac_address]['node_mac']
            if old_node_mac != node_mac:
                node_devices[old_node_mac].discard(mac_address)
                throttled_device_count_update(old_node_mac, len(node_devices[old_node_mac]))
            
        if node_mac not in node_devices:
            node_devices[node_mac] = set()
        node_devices[node_mac].add(mac_address)
        throttled_device_count_update(node_mac, len(node_devices[node_mac]))

        return jsonify({"status": "success", "message": "Data received"})
    except KeyError as e:
        print(f"Missing key in data: {str(e)}")
        return jsonify({"status": "error", "message": "Missing key in data"}), 400
    except Exception as e:
        print(f"Error in /api/data route: {str(e)}")
        return jsonify({"status": "error", "message": "Internal Server Error"}), 500

@app.route('/api/ble_data', methods=['POST'])
def receive_ble_data():
    data = request.get_json()
    
    for device in data:
        mac_address = device['mac_address']
        rssi = device['rssi']
        
        watch_nodes[mac_address] = {'type': 'BLE', 'rssi': rssi}
        throttled_tag_update(mac_address, 'BLE', rssi, None)

        node_mac = device['boardName']
        if node_mac not in node_devices:
            node_devices[node_mac] = set()
        node_devices[node_mac].add(mac_address)
        throttled_device_count_update(node_mac, len(node_devices[node_mac]))
    
    return jsonify({"status": "success", "message": "BLE data received"})

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)