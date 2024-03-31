from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

# In-memory storage for device counts
indoor_devices = 0
outdoor_devices = 0

@app.route('/')
def dashboard():
    return render_template('dashboard.html')

@app.route('/api/indoor', methods=['POST'])
def update_indoor_devices():
    global indoor_devices
    data = request.get_json()
    indoor_devices = data['count']
    return jsonify({'status': 'success'})

@app.route('/api/outdoor', methods=['POST'])
def update_outdoor_devices():
    global outdoor_devices
    data = request.get_json()
    outdoor_devices = data['count']
    return jsonify({'status': 'success'})

@app.route('/api/devices')
def get_device_counts():
    return jsonify({'indoor': indoor_devices, 'outdoor': outdoor_devices})

if __name__ == '__main__':
    app.run()