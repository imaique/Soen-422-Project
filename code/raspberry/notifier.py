import config
import socket
import bluetooth

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
        if abs(self.distances[angle] - distance) > 0.3:
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


try:
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((server_address, server_port))
    clf = Classifier()
    print("Connected to the server")

    while True:
        # Receive data from the server
        data = client_socket.recv(
            8
        )  # Assuming 8 bytes of data (4 for distance and 4 for angle)
        distance = int.from_bytes(data[:4], byteorder="big")
        angle = int.from_bytes(data[4:], byteorder="big")
        clf.add_distance(distance, angle)


except socket.error as e:
    print("Error:", e)

finally:
    client_socket.close()
