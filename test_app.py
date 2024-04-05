import requests
import json
import time
import random

# Flask server URL
flask_url = 'http://localhost:5000'

while True:
    # Simulate LoRa device data
    lora_data = {
        "mac": "aa:bb:cc:dd:ee:ff",
        "rssi": random.randint(-50, -20),
        "timestamp": int(time.time())
    }

    # Send LoRa device data to the server
    response = requests.post(flask_url + "/api/data", json=lora_data)
    print("LoRa device data sent. Response:", response.json())

    # Simulate BLE Mesh device data
    ble_data = [
        {
            "mac_address": "11:22:33:44:55:66",
            "rssi": random.randint(-50, -20),
            "boardName": "Board1"
        },
        {
            "mac_address": "77:88:99:aa:bb:cc",
            "rssi": random.randint(-50, -20),
            "boardName": "Board2"
        }
    ]

    # Send BLE Mesh device data to the server
    response = requests.post(flask_url + "/api/ble_data", json=ble_data)
    print("BLE Mesh device data sent. Response:", response.json())

    # Wait for a certain interval before sending the next data
    time.sleep(5)  # Adjust the interval as needed