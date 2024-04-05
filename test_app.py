import pytest
import requests
import json
from flask_socketio import SocketIO

@pytest.fixture(scope="module")
def test_client():
    from app import app
    with app.test_client() as testing_client:
        with app.app_context():
            yield testing_client

@pytest.fixture(scope="module")
def socketio_client(request):
    from app import app, socketio
    client = SocketIO(app, flask_test_client=app.test_client())
    def fin():
        client.disconnect()
    request.addfinalizer(fin)
    return client

def test_lora_device_data(test_client, socketio_client):
    # Simulate LoRa device data
    lora_data = {
        "mac": "aa:bb:cc:dd:ee:ff",
        "rssi": -20,
        "timestamp": 1234567890
    }

    # Send LoRa device data to the server
    response = test_client.post("/api/data", json=lora_data)
    assert response.status_code == 200
    assert response.json["status"] == "success"

    # Check if the device data is processed correctly
    assert "aa:bb:cc:dd:ee:ff" in indoor_devices
    assert len(indoor_devices) == 1

    # Check if the socketio events are emitted correctly
    socketio_client.connect()
    socketio_client.emit("device_update", callback=True)
    socketio_client.emit("signal_update", callback=True)
    # Add assertions for the expected behavior of the frontend dashboard

def test_ble_mesh_device_data(test_client, socketio_client):
    # Simulate BLE Mesh device data
    ble_data = [
        {
            "mac_address": "11:22:33:44:55:66",
            "rssi": -25,
            "boardName": "Board1"
        },
        {
            "mac_address": "77:88:99:aa:bb:cc",
            "rssi": -40,
            "boardName": "Board2"
        }
    ]

    # Send BLE Mesh device data to the server
    response = test_client.post("/api/ble_data", json=ble_data)
    assert response.status_code == 200
    assert response.json["status"] == "success"

    # Check if the device data is processed correctly
    assert "11:22:33:44:55:66" in ble_devices
    assert "77:88:99:aa:bb:cc" not in ble_devices
    assert len(ble_devices) == 1

    # Check if the socketio events are emitted correctly
    socketio_client.connect()
    socketio_client.emit("ble_device_update", callback=True)
    socketio_client.emit("ble_signal_update", callback=True)
    # Add assertions for the expected behavior of the frontend dashboard