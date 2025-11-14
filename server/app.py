from flask import Flask

app = Flask(__name__)


@app.route('/')
def index():
    return 'Hello — Flask is installed and working!'


if __name__ == '__main__':
    # listen on all interfaces so you can access it from other machines on the LAN
    app.run(host='0.0.0.0', port=5000)
