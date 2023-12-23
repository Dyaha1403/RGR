import socket
from flask import Flask, request, render_template
from flask_cors import CORS
from flask_sslify import SSLify

app = Flask(__name__)
CORS(app)
sslify = SSLify(app)

# Создаем сокет для взаимодействия с C++ сервером
cpp_server_host = '127.0.0.1'  # Замените на IP-адрес C++ сервера
cpp_server_port = 6000          # Замените на порт C++ сервера
cpp_server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    cpp_server_socket.connect((cpp_server_host, cpp_server_port))
    print(f"Connected to C++ server at {cpp_server_host}:{cpp_server_port}")
except Exception as e:
    print(f"Failed to connect to C++ server: {e}")
    exit()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/motion_data', methods=['POST'])
def receive_motion_data():
    try:
        data = request.json
        # Обработка полученных данных, например, отправка их в код на C++
        print("Received motion data:", data)
        
        # Преобразование данных в строку и отправка на C++ сервер
        motion_data_str = str(data)
        cpp_server_socket.sendall(motion_data_str.encode())

        return 'OK'
    except Exception as e:
        print(f"Error processing motion data: {e}")
        return 'Error'

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=443, threaded=True, ssl_context=('C:\\Certbot\\live\\d.kikker.online\\fullchain.pem', 'C:\\Certbot\\live\\d.kikker.online\\privkey.pem'))
