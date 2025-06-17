from flask import Blueprint, render_template, request
from flask_socketio import emit, disconnect
from app import socketio
import json

main = Blueprint('main', __name__)

# Store connected clients with their transport info
connected_clients = {}

@main.route('/')
def index():
    return render_template('index.html')

@socketio.on('connect')
def handle_connect():
    print(f"Client connected: {request.sid}")
    connected_clients[request.sid] = {
        'sid': request.sid,
        'transport': 'websocket'
    }
    print(f"Total connected clients: {len(connected_clients)}")

@socketio.on('disconnect')
def handle_disconnect():
    print(f"Client disconnected: {request.sid}")
    connected_clients.pop(request.sid, None)
    print(f"Total connected clients: {len(connected_clients)}")

@socketio.on('send_message')
def handle_message(data):
    message = data.get('message', '')
    print(f"Received message from {request.sid}: {message}")
    broadcast_message(message)

def broadcast_message(message):
    """Send raw Socket.IO packet to all clients"""
    print(f"Broadcasting message: {message}")
    
    # Create the Socket.IO event packet manually
    event_data = json.dumps(["display_message", {"message": message}])
    socket_io_packet = f"42{event_data}"
    
    print(f"Raw packet to send: {socket_io_packet}")
    print(f"Connected clients: {list(connected_clients.keys())}")
    
    # Send to each client using the underlying engine
    for client_id in list(connected_clients.keys()):
        try:
            socketio.server.eio.send(client_id, socket_io_packet)
            print(f"Raw packet sent to {client_id}")
        except Exception as e:
            print(f"Error sending to {client_id}: {e}")
            connected_clients.pop(client_id, None)

@main.route('/send/<message>')
def send_direct_message(message):
    print(f"Direct message send: {message}")
    broadcast_message(message)
    return f"Message '{message}' sent to {len(connected_clients)} clients"

@main.route('/clients')
def list_clients():
    return f"Connected clients: {len(connected_clients)} - {list(connected_clients.keys())}"

@main.route('/raw_send/<message>')
def raw_send(message):
    """Send message using different methods"""
    print(f"Trying to send: {message}")
    print(f"Connected clients: {list(connected_clients.keys())}")
    
    success_count = 0
    
    for client_id in list(connected_clients.keys()):
        print(f"Attempting to send to client: {client_id}")
        
        # Method 1: Try direct emit to room
        try:
            socketio.emit('display_message', {'message': message}, room=client_id)
            print(f"✅ Method 1 (emit to room) succeeded for {client_id}")
            success_count += 1
        except Exception as e:
            print(f"❌ Method 1 failed for {client_id}: {e}")
            
            # Method 2: Try raw packet send
            try:
                packet = f'42["display_message",{{"message":"{message}"}}]'
                socketio.server.eio.send(client_id, packet)
                print(f"✅ Method 2 (raw packet) succeeded for {client_id}")
                success_count += 1
            except Exception as e2:
                print(f"❌ Method 2 also failed for {client_id}: {e2}")
                
                # Method 3: Try broadcast with namespace
                try:
                    socketio.emit('display_message', {'message': message}, namespace='/')
                    print(f"✅ Method 3 (namespace broadcast) attempted")
                    success_count += 1
                    break  # Only do this once
                except Exception as e3:
                    print(f"❌ Method 3 failed: {e3}")
    
    return f"Attempted to send '{message}' - {success_count} successes out of {len(connected_clients)} clients"