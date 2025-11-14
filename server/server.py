# Save as server.py on your Raspberry Pi
import os
import subprocess
from datetime import datetime
from flask import Flask, request, jsonify, render_template

app = Flask(__name__)

# Store alerts in memory (in production, use a database)
alerts_log = []


def show_fire_alarm_notification(message_data):
    """Display a desktop notification for fire alarm using zenity GUI dialog."""
    try:
        # Extract message details if available
        msg = message_data.get('msg', 'Fire detected!') if isinstance(message_data, dict) else 'Fire detected!'
        sensor = message_data.get('sensor', 'Unknown') if isinstance(message_data, dict) else 'Unknown'
        
        # Get DISPLAY environment variable for GUI apps
        display = os.environ.get('DISPLAY', ':0')
        user = os.environ.get('USER', 'abadesha')
        
        # Use zenity for a popup dialog (more reliable than notify-send on this system)
        try:
            env = os.environ.copy()
            env['DISPLAY'] = display
            
            # Run zenity in the background so it doesn't block the HTTP response
            subprocess.Popen([
                'zenity',
                '--error',
                '--title=🔥 FIRE ALARM TRIGGERED 🔥',
                f'--text=<span font="14" weight="bold">FIRE ALARM DETECTED!</span>\n\n{msg}\n\nSensor: {sensor}\n\nPlease check the fire detection system immediately!',
                '--width=450',
                '--height=200'
            ], env=env, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            
            app.logger.info(f'Fire alarm popup displayed for: {msg}')
            print(f"🔥 FIRE ALARM POPUP SHOWN: {msg} (Sensor: {sensor})")
        except Exception as dialog_err:
            app.logger.error(f'Failed to show dialog: {dialog_err}')
            print(f"⚠️  Failed to show GUI alert: {dialog_err}")
                
    except Exception as e:
        app.logger.error(f'Failed to send notification: {e}')
        print(f"⚠️  Notification error: {e}")


@app.route('/')
def index():
    """Render the main dashboard page."""
    return render_template('index.html')


@app.route('/api/alerts')
def get_alerts():
    """API endpoint to get all alerts."""
    return jsonify(alerts=alerts_log)


@app.route('/alert', methods=['POST'])
def alert():
    data = request.get_json()
    
    # Add timestamp and store alert
    alert_record = {
        'message': data.get('msg', 'Fire detected!'),
        'sensor': data.get('sensor', 'Unknown'),
        'timestamp': datetime.now().isoformat()
    }
    alerts_log.append(alert_record)
    
    # Keep only last 100 alerts
    if len(alerts_log) > 100:
        alerts_log.pop(0)
    
    # log to Flask logger and stdout for visibility
    app.logger.info('Alert received: %s', data)
    print(f"🔥 FIRE ALARM: Alert received: {data}")
    
    # Show desktop notification
    show_fire_alarm_notification(data)
    
    return jsonify(status="ok"), 200


if __name__ == '__main__':
    # allow overriding the port via environment variable (e.g., PORT=5001)
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port)