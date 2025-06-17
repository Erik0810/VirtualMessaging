from gevent import monkey
monkey.patch_all()

from app import create_app, socketio

app = create_app()

if __name__ == '__main__':
    # Using gevent as the async server
    socketio.run(app, 
                host='0.0.0.0',  # Allow external connections
                port=5000,       # Port can be changed if needed
                debug=True)      # Set to False in production