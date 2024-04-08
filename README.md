# Project Overview
This project focuses on developing a watch tracker system with a monitoring dashboard, utilizing a combination of BLE and LoRa technologies for data transmission. The system includes BLE mesh nodes, LoRa nodes, a gateway node, and a Flask-based web server with Socket.IO for real-time updates.

## System Components:
- BLE Mesh:
   - mesh.ino: Implements a BLE mesh network using PainlessMesh. Scans for BLE devices and transmits the collected data (MAC addresses and RSSI values) through the mesh network.
   - process.py: Processes data received from the BLE mesh network via serial communication and sends it to the Flask server.
- BLE Node:
   - ble_node.ino: Individual BLE node that scans for specific watch MAC addresses and sends data to the web server via WiFi and HTTP POST requests.
- Gateway Node:
   - gateway_node.ino: Receives data from LoRa nodes and transmits it to the Flask server via WiFi and HTTP POST requests.
   - boards.h and utilities.h: Header files containing board-specific configurations and utility functions.
- LoRa Transmit:
   - lora_transmit.ino: Individual LoRa node that transmits its own MAC address periodically.
   - boards.h and utilities.h: Header files containing board-specific configurations and utility functions.
- Web Server:
   - app.py: Flask application with Socket.IO functionality. Processes received data from BLE and LoRa nodes, determines device locations, and sends real-time updates to the dashboard.
   - templates/dashboard.html: HTML template for the monitoring dashboard, visualizing device counts and signal strengths using Chart.js.
- Watch Tracker Dashboard: (React application)
   - Displays real-time data received from the server, including node locations, device counts, and signal strengths.
- Test Scripts:
   - test_app.py: Simulates data transmission from watch devices to the server via HTTP POST requests.
   - watch-tracker-server/test_server.js: Simulates watch connections to different nodes and sends data to the server.
   - watch-tracker-server/test_server2.js: Simulates a watch moving through a sequence of nodes.

## Project Setup:
- Install required dependencies for each component (Python libraries for the server and Node.js packages for the dashboard).
- Configure the server URL and other parameters in the relevant files.
- Upload the firmware to the respective nodes (BLE mesh, BLE node, LoRa node, and gateway node).
- Start the Flask server and the React application.
- Run the test scripts to simulate device data transmission and observe the dashboard updates.

## Project Features:
- Real-time tracking of watch device locations based on signal strengths.
- Visual representation of BLE and LoRa nodes on the dashboard.
- Display of device counts within the range of each node.
- Connections log displaying connection events.

## Future Improvements:
- Enhance the dashboard with interactive features, such as filtering and searching for specific devices.
- Implement historical data storage and visualization.
- Integrate additional sensors and functionalities into the system.
- Improve the robustness and security of data transmission and processing.

## Conclusion
This project provides a basic framework for a watch tracker system using BLE and LoRa technologies. The implementation demonstrates the potential for real-time monitoring and visualization of IoT devices in both indoor and outdoor environments.
