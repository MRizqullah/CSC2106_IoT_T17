import requests
import json
import time
import random
import threading

# Flask server URL
flask_url = 'http://localhost:5000'

# Simulated watch MAC addresses (reduced to 2)
watch_macs = ['aa:bb:cc:dd:ee:ff', 'ff:ee:dd:cc:bb:aa']

# Simulated LoRa nodes (reduced to 1)
lora_nodes = [{'mac': '11:22:33:44:55:66', 'rssi_range': (-50, -20)}]

# Simulated BLE Mesh nodes (reduced to 1)
ble_nodes = [{'mac': 'ff:ee:dd:cc:bb:aa', 'rssi_range': (-40, -10)}]

def simulate_watch_connection(watch_mac):
    while True:
        node_type = random.choice(['LoRa', 'BLE'])
        if node_type == 'LoRa':
            node = random.choice(lora_nodes)
            rssi = random.randint(node['rssi_range'][0], node['rssi_range'][1])
            lora_data = {
                'mac': watch_mac,
                'rssi': rssi,
                'timestamp': int(time.time()),
                'node_mac': node['mac']
            }
            response = requests.post(flask_url + '/api/data', json=lora_data)
            print(f"Watch {watch_mac} connected to LoRa node {node['mac']}. RSSI: {rssi}. Response: {response.json()}")
        else:
            node = random.choice(ble_nodes)
            rssi = random.randint(node['rssi_range'][0], node['rssi_range'][1])
            ble_data = [
                {
                    'mac_address': watch_mac,
                    'rssi': rssi,
                    'boardName': node['mac']
                }
            ]
            response = requests.post(flask_url + '/api/ble_data', json=ble_data)
            print(f"Watch {watch_mac} connected to BLE node {node['mac']}. RSSI: {rssi}. Response: {response.json()}")

        # Increased sleep time to reduce frequency of updates
        time.sleep(random.uniform(3, 10))

if __name__ == '__main__':
    threads = []
    # Limit the number of threads to 2
    for watch_mac in watch_macs[:2]:
        thread = threading.Thread(target=simulate_watch_connection, args=(watch_mac,))
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()
