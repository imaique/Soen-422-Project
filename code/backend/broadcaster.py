import socket
import threading
import config
import queue


broadcast_address = config.SERVER_ADDRESS
port = config.PORT
serial_port = config.SERIAL_PORT

# Global variables for distance and angle
messages = queue.Queue()
object_update = queue.Queue()


# Function to update distance and angle from serial
def get_sonar():
    SONAR_PORT = 80
    # Create a TCP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(("0.0.0.0", SONAR_PORT))  # Bind to localhost and the specified port
    sock.listen(1)  # Listen for incoming connections with a backlog of 5

    while True:
        print(f"Waiting for connections on port {SONAR_PORT}...")
        client, addr = sock.accept()
        print("Connected by", addr)
        handle_sonar_connection(client)


def handle_sonar_connection(client_socket: socket.socket):
    client_socket.settimeout(5.0)
    try:
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            try:
                line = data.decode("utf-8").strip()
                if line.startswith("da"):
                    [distance, angle] = [int(x) for x in line[2:].split(",")]

                    # Acquire the condition lock and update the values
                    messages.put((distance, angle))
                    print("Received:", line)

                if not object_update.empty():
                    client_socket.sendall(b"Hello from Python")

            except UnicodeDecodeError as e:
                print(f"caught: {e}. Skipping")

    except socket.timeout:
        print("Connection timed out. Closing connection.")
    finally:
        client_socket.close()


# Function to handle a client's connection
def handle_client(client_socket: socket, address):
    print(f"Connected to {address}")

    while True:
        try:
            # Wait for an update in the data
            (distance, angle) = messages.get()
            data = distance.to_bytes(4, byteorder="big") + angle.to_bytes(
                4, byteorder="big"
            )
            client_socket.send(data)
            messages.task_done()

            print(f"Sent message to {address}")
        except socket.error as e:
            print("Error:", e)
            break

    # Close the connection when done
    client_socket.close()


def listen():
    # Create a TCP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(("localhost", port))  # Bind to localhost and the specified port
    sock.listen(5)  # Listen for incoming connections with a backlog of 5

    print(f"Waiting for connections on port {port}...")

    while True:
        print("running")
        client, addr = sock.accept()  # Accept a new connection
        client_thread = threading.Thread(
            target=handle_client, args=(client, addr), daemon=True
        )
        client_thread.start()


# Start the serial thread
sonar_thread = threading.Thread(target=get_sonar, daemon=True)
sonar_thread.start()

listener_thread = threading.Thread(target=listen, daemon=True)
listener_thread.start()

while True:
    pass
