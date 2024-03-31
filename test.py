from socketio.client import Client

# Initialize the socket.io client
socket = Client()

# Define the event handler for the 'connect' event
@socket.event
def connect():
    print('Connected to server')

# Define the event handler for the 'device_signal' event
@socket.event
def device_signal(data):
    print('Received device signal:', data)

# Connect to the server
socket.connect('http://localhost:5000')  # Replace with your server's URL

# Emit the 'device_signal' event with the data
socket.emit('device_signal', {
    'device_id': '123',
    'indoor_signal': 80,
    'outdoor_signal': 100
})

# Wait for events indefinitely
socket.wait()
