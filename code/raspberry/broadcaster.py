import socket
import serial
import time

# broadcast address localhost (127.0.0.1)
broadcast_address = "127.0.0.1"
port = 12345  # port number
serial_port = "COM5"


# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

# TODO: Add logic to wait until port is busy before starting to listen
ser = serial.Serial(serial_port, 115200)


# Data to be sent
while True:
    if ser.in_waiting:
        line = ser.readline().decode().strip()
        if line.startswith("da"):
            [distance, angle] = [int(x) for x in line[2:].split(",")]
            print("Received:", line)
            try:
                # Send the broadcast message
                data = distance.to_bytes(4, byteorder="big") + angle.to_bytes(
                    4, byteorder="big"
                )
                sock.sendto(data, (broadcast_address, port))
                print(f"Broadcasted message to {broadcast_address}:{port}")
            except socket.error as e:
                print("Error:", e)
