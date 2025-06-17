from flask import Blueprint, render_template
from flask_socketio import emit
from app import socketio

main = Blueprint('main', __name__)

from flask import Blueprint, render_template, request
from flask_socketio import emit
from app import socketio
import time

main = Blueprint('main', __name__)

@main.route('/')
def index():
    return render_template('index.html')

@socketio.on('connect')
def handle_connect():
    print(f"Client connected: {request.sid}")
    # Send a welcome message to confirm connection
    emit('display_message', {'message': 'Connected!'}, room=request.sid)

@socketio.on('disconnect')
def handle_disconnect():
    print(f"Client disconnected: {request.sid}")

@socketio.on('send_message')
def handle_message(data):
    message = data.get('message', '')
    print(f"Received message: {message}")
    print(f"Client SID: {request.sid}")
    
    # Adding a small delay to ensure the message is processed before sending
    time.sleep(0.1)
    
    # Create the event data
    event_data = {'message': message}
    print(f"Broadcasting event data: {event_data}")
    
    # Broadcast to all connected clients (add privacy features later?)
    emit('display_message', event_data, broadcast=True)
    print("Message broadcast complete")

# Alternative endpoint to send messages directly (for testing)
@main.route('/send/<message>')
def send_direct_message(message):
    print(f"Direct message send: {message}")
    socketio.emit('display_message', {'message': message}, broadcast=True)
    return f"Message '{message}' sent to all clients"