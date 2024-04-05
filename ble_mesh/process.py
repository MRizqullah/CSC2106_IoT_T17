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
fastapi_url = 'http://localhost:8000/message'

while True:
    if ser.in_waiting > 0:
        # Read the incoming data
        data = ser.readline().decode('utf-8').strip()
        
        try:
            # Parse the JSON data
            json_data = json.loads(data)
            
            # Process the received data
            try:
                for device in json_data:
                    mac_address = device['mac_address']
                    rssi = device['rssi']
                    board_name = device['boardName']
                    
                    if mac_address == target_mac_address:
                        message = f"Smart_Watch IS in Area of Board {board_name} with RSSI Strength {rssi}"
                        print(message)
                        
                        # Send the message back to the serial port
                        ser.write((message + '\n').encode('utf-8'))
                        
                        # Make a POST request to the FastAPI server
                        response = requests.post(fastapi_url, json={'message': message})
                        print("FastAPI response:", response.json())
            except TypeError:
                # Ignore non-iterable objects
                pass
        
        except json.JSONDecodeError:
            print("Invalid JSON data received.")