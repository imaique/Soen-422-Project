import socket


def receive_data():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(("localhost", 1234))  # Binds to localhost on port 1234
    server_socket.listen(1)  # Listens for incoming connections
    print("IP Address: ", get_ip_address())
    print("Listening on port 1234...")

    while True:
        (
            client_socket,
            client_address,
        ) = server_socket.accept()  # Accepts incoming connection
        print(f"Connection established with {client_address}")

        data = client_socket.recv(1024).decode()  # Receives data from the client
        if not data:
            break

        try:
            # Split the received data into two integers
            [num1, num2] = data
            print(f"Received integers: {num1} and {num2}")
        except ValueError:
            print("Invalid data format. Expected two integers separated by a space.")

        client_socket.close()  # Closes the connection with the client


def get_ip_address():
    hostname = socket.gethostname()
    ip_address = socket.gethostbyname(hostname)
    return ip_address


if __name__ == "__main__":
    receive_data()
