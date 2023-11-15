import socket
import serial
import threading
import config

broadcast_address = config.SERVER_ADDRESS
port = config.PORT 
serial_port = config.SERIAL_PORT

# Global variables for distance and angle
distance = 0
angle = 0

# Create a threading condition object
condition = threading.Condition()


# Function to update distance and angle from serial
def get_serial():
    global distance, angle
    ser = serial.Serial(serial_port, 115200)

    while True:
        if ser.in_waiting:
            try:
                
                line = ser.readline().decode('utf-8').strip()
                if line.startswith("da"):
                    [dist, ang] = [int(x) for x in line[2:].split(",")]

                    # Acquire the condition lock and update the values
                    with condition:
                        distance = dist
                        angle = ang
                        # Notify all waiting threads that new data is available
                        condition.notify_all()

                    print("Received:", line)
            except UnicodeDecodeError as e:
                print(f'caught: {e}. Skipping')


# Function to handle a client's connection
def handle_client(client_socket: socket, address):
    print(f"Connected to {address}")

    while True:
        try:
            # Wait for an update in the data
            with condition:
                condition.wait()

                # Send the message over the client's socket
                data = distance.to_bytes(4, byteorder="big") + angle.to_bytes(
                    4, byteorder="big"
                )
                client_socket.send(data)

            print(f"Sent message to {address}")
        except socket.error as e:
            print("Error:", e)
            break

    # Close the connection when done
    client_socket.close()


# Start the serial thread
serial_thread = threading.Thread(target=get_serial)
serial_thread.start()

# Create a TCP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(("localhost", port))  # Bind to localhost and the specified port
sock.listen(5)  # Listen for incoming connections with a backlog of 5

print(f"Waiting for connections on port {port}...")

while True:
    client, addr = sock.accept()  # Accept a new connection
    client_thread = threading.Thread(target=handle_client, args=(client, addr))
    client_thread.start()
