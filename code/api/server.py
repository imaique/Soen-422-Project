import socket
import struct
import time


def receive_data():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server_socket.connect(("192.168.132.73", 1234))

    data_to_send = "Hello from Python!"

    while True:
        # Send data to Arduino
        server_socket.send(data_to_send.encode())
        print("Data sent to Arduino: {}".format(data_to_send))
        [distance, angle], addr = server_socket.recvfrom(8)
        print(f"Received: Angle={angle}, Distance={distance}")
        time.sleep(0.1)


if __name__ == "__main__":
    receive_data()
