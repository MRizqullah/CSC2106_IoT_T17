# IoT Device Monitoring Dashboard

This project is a monitoring dashboard for IoT devices that tracks the location and signal strengths of devices in indoor and outdoor perimeters. The dashboard provides real-time visualizations of device counts and signal strengths using a combination of Flask, Flask-SocketIO, and Chart.js.

![Alt text](https://i.imgur.com/TUF74PO.png)

## Features

- Real-time updates of device counts in indoor and outdoor perimeters
- Visual representation of perimeters using circles
- Line chart displaying the signal strengths of indoor and outdoor nodes over time
- Server-side logic for determining device location based on signal strengths
- Support for IoT devices to send signal data to the server via HTTP POST requests

## Prerequisites

- Python 3.x
- Flask
- Flask-SocketIO
- pytest

## Installation

1. Clone the repository:

   ```
   git clone https://github.com/MRizqullah/CSC2106_IoT_T17.git
   ```

2. Navigate to the project directory:

   ```
   cd CSC2106_IoT_T17
   ```

3. Create a virtual environment (optional but recommended):

   ```
   python -m venv venv
   ```

4. Activate the virtual environment:

   - For Windows:

     ```
     venv\Scripts\activate
     ```

   - For macOS and Linux:

     ```
     source venv/bin/activate
     ```

5. Install the required dependencies:

   ```
   pip install -r requirements.txt
   ```

## Usage

1. Start the Flask server:

   ```
   python app.py
   ```

2. Open a web browser and navigate to `http://localhost:5000` to access the monitoring dashboard.

3. To simulate IoT devices sending signal data to the server, you can use the provided example script or create your own client-side script. Run the script to start sending data:

   ```
   pytest test_app.py
   ```

4. The dashboard will update in real-time, displaying the device counts in the circles and the signal strengths in the line chart.

## Configuration

- If you want to change the server's host or port, modify the following line in `app.py`:

  ```python
  socketio.run(app, host='0.0.0.0', port=5000)
  ```

- If you're running the server on a different machine or network, update the `SERVER_URL` in the IoT device simulator script (`iot_device_simulator.py`) to match the server's IP address and port.

## File Structure

- `app.py`: The main Flask server file that handles routes, Socket.IO events, and device signal processing.
- `templates/dashboard.html`: The HTML template for the monitoring dashboard.
- `iot_device_simulator.py`: An example script to simulate IoT devices sending signal data to the server.
- `requirements.txt`: The list of required Python dependencies.
- `README.md`: This readme file.

## Contributing

Contributions are welcome! If you find any issues or have suggestions for improvements, please open an issue or submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).
