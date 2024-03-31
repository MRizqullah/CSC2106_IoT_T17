from flask import Flask, render_template, request, jsonify
from flask_socketio import SocketIO, emit

app = Flask(__name__)
app.config['SECRET_KEY'] = 'your-secret-key'
socketio = SocketIO(app)

indoor_devices = 0
outdoor_devices = 0

@app.route('/')
def dashboard():
    return render_template('dashboard.html')

@socketio.on('connect')
def handle_connect():
    emit('device_counts', {'indoor': indoor_devices, 'outdoor': outdoor_devices})

@socketio.on('update_indoor')
def handle_update_indoor(data):
    global indoor_devices
    indoor_devices = data['count']
    emit('device_counts', {'indoor': indoor_devices, 'outdoor': outdoor_devices}, broadcast=True)

@socketio.on('update_outdoor')
def handle_update_outdoor(data):
    global outdoor_devices
    outdoor_devices = data['count']
    emit('device_counts', {'indoor': indoor_devices, 'outdoor': outdoor_devices}, broadcast=True)

if __name__ == '__main__':
    socketio.run(app)