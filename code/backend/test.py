import socket


def handle_client_connection(client_socket: socket.socket):
    client_socket.settimeout(5.0)
    try:
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            print("Received:", data.decode())
            client_socket.sendall(b"Hello from Python")
    except socket.timeout:
        print("Connection timed out. Closing connection.")
    finally:
        client_socket.close()


server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_socket.bind(("0.0.0.0", 80))
server_socket.listen(1)

while True:
    print("Waiting for a connection...")
    conn, addr = server_socket.accept()
    print("Connected by", addr)
    handle_client_connection(conn)
