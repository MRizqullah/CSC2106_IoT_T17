import requests
import time

SERVER_URL = 'http://YOUR_LAPTOP_IP:5000'

while True:
    # Simulate signal strengths (replace with actual sensor data)
    indoor_signal = random.randint(0, 100)
    outdoor_signal = random.randint(0, 100)

    # Send the signal strengths to the server
    data = {
        'device_id': 'device123',
        'indoor_signal': indoor_signal,
        'outdoor_signal': outdoor_signal
    }
    requests.post(SERVER_URL + '/device_signal', json=data)

    time.sleep(5)  # Send data every 5 seconds