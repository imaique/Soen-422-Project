import socket
import serial
import threading
import config
import bluetooth
import time
import config

server_address = config.SERVER_ADDRESS
server_port = config.PORT

MAX_DISTANCE = 50


class Classifier:
    def __init__(self) -> None:
        self.distances = [None] * 180
        self.corner_bump = 0
        self.code_red = False
        self.code_red_sweep = False
        self.code_green = False
        self.code_green_sweep = False

    def add_distance(self, distance, angle):
        if self.fully_swept() and abs(self.distances[angle] - distance) > 0.3:
            self.code_red_sweep = True
            self.code_red = True
        if angle == 180 or angle == 1:
            if not self.fully_swept():
                self.distances[angle] = distance
                self.corner_bump += 1
            else:
                self.code_red = self.code_red_sweep
                self.code_red_sweep = False

    def fully_swept(self):
        return self.corner_bump == 2
    
    def time_to_send_update(self):
        return False
    
    def get_data(self):

        object_type = 0 # 0 friendly 1 unknown'
        distance = 50
        angle = 20
        data =  object_type.to_bytes(4, byteorder="big") + distance.to_bytes(4, byteorder="big") + angle.to_bytes(
                4, byteorder="big"
        )
        return data


broadcast_address = config.SERVER_ADDRESS
port = config.PORT 
serial_port = config.SERIAL_PORT

# Create a threading condition object
condition = threading.Condition()

# to be removed
object_type = 0 # 0 friendly 1 unknown'
distance = 50
angle = 20
data = f'{object_type},{distance},{angle},'


# Function to update distance and angle from serial
def get_broadcast():

    global clf
    clf = Classifier()

    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((server_address, server_port))
        
        print("Connected to the server")

        while True:
            # Receive data from the server
            data = client_socket.recv(
                8
            )  # Assuming 8 bytes of data (4 for distance and 4 for angle)
            distance = int.from_bytes(data[:4], byteorder="big")
            angle = int.from_bytes(data[4:], byteorder="big")
            with condition:
                clf.add_distance(distance, angle)
                if clf.time_to_send_update():
                    condition.notify_all()

    except socket.error as e:
        print("Error:", e)

    finally:
        client_socket.close()

# Function to handle a client's connection
def handle_listener(client_socket: socket, address):
    print(f"Connected to {address}")

    while True:
        try:
            # Wait for an update in the data
            with condition:
                condition.wait()

                # Send the message over the client's socket
                client_socket.send(data)
                print(f"Sent message to {address}")


        except Exception as e:
            print("Error:", e)
            break

    # Close the connection when done
    connected_devices.remove(address)
    client_socket.close()


# Start the broadcast listerner thread
broadcast_listener = threading.Thread(target=get_broadcast)
broadcast_listener.start()

connected_devices = set()
registered_listeners = [{
    "name": "Alarm Arduino",
    "address": 'C8:C9:A3:FB:F6:8A',
    "port": 1
}]
is_clear = True
while True:
    # Check for registered devices
    for registered_device in registered_listeners:
        addr = registered_device['address']
        if addr in connected_devices:
            print("already in here")
            continue
        name = registered_device['name']
        port = registered_device['port']

        listener_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
        listener_sock.connect((addr, port))
        connected_devices.add(addr)
        client_thread = threading.Thread(target=handle_listener, args=(listener_sock, addr))
        client_thread.start()
    time.sleep(10)
    # bluetooth test code to be removed
    with condition:
        if is_clear:
            object_type = 2 # 0 friendly 1 unknown'
            distance = 50
            angle = 20
            data = f'{object_type},{distance},{angle},'


        else:
            object_type = 1 # 0 friendly 1 unknown'
            distance = 50
            angle = 20
            data = f'{object_type},{distance},{angle},'
            
        is_clear = not is_clear
        condition.notify_all()

    

