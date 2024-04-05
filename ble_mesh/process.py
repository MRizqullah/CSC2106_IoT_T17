import serial
import json
import requests

# Configure the serial connection
serial_port = 'COM6'  # Replace with the appropriate serial port
baud_rate = 115200

# Open the serial connection
ser = serial.Serial(serial_port, baud_rate)

# MAC address to look for
target_mac_address = 'dc:54:75:e4:7a:e1'

# FastAPI server URL
# fastapi_url = 'http://localhost:8000/message'

# Flask server URL
flask_url = 'http://localhost:5000/api/ble_data'

while True:
    if ser.in_waiting > 0:
        # Read the incoming data
        data = ser.readline().decode('utf-8').strip()
        
        try:
            # Parse the JSON data
            json_data = json.loads(data)
            
            # Send the JSON data to the Flask server
            response = requests.post(flask_url, json=json_data)
            print("Flask server response:", response.json())
        
        except json.JSONDecodeError:
            print("Invalid JSON data received.")