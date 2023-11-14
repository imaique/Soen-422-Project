import json

with open("config.json", "r") as f:
    config = json.load(f)

PORT = config["PORT"]
SERVER_ADDRESS = config["ADDRESS"]
SERIAL_PORT = config["SERIAL_PORT"]
