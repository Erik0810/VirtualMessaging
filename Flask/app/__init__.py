from flask import Flask
from flask_socketio import SocketIO
from dotenv import load_dotenv
import os

# Load environment variables
load_dotenv()

socketio = SocketIO(logger=True, engineio_logger=True)

def create_app():
    app = Flask(__name__)
    app.config['SECRET_KEY'] = os.getenv('SECRET_KEY')
    
    # Initialize Flask-SocketIO
    socketio.init_app(app,
                     cors_allowed_origins="*",
                     async_mode='threading')
    
    # Register blueprints (this isnt necessary right now, but nice for future scaling)
    from app.routes import main
    app.register_blueprint(main)
    
    return app