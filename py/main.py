import serial
import time
import hashlib
from prompt_toolkit import prompt
from prompt_toolkit.completion import WordCompleter
from serial.tools import list_ports

def main():
    # List all available serial ports and let the user select one
    ports = list_ports.comports()
    port_names = [port.device for port in ports]
    print("-".join([" " for _ in range(10)]))
    print("\n".join(port_names))
    print("-".join([" " for _ in range(10)]))
    port_completer = WordCompleter(port_names)
    port_name = prompt("Enter port name: ", completer=port_completer)

    # Initialize serial connection
    ser = serial.Serial(port_name, 115200)  # adjust baud rate as necessary
    commands = ['init', 'version', 'open', 'erase', 'sign', 'restore', 'test', 'rnd', 'rea']
    print("-".join([" " for _ in range(10)]))
    print("\n".join(commands))
    print("-".join([" " for _ in range(10)]))
    command_completer = WordCompleter(commands)

    while True:
        command = prompt("Enter command: ", completer=command_completer)
        if not (command in commands):
            print("command",command,"not found")
            continue

        # Send the command
        ser.write((command + '\n').encode())
        # If the command requires a second part (like a password or hash), send it
        if command in {"init", "open"}:
            password = prompt("Enter password: ", is_password=True)
            passwordbytes = hashlib.sha256(password.encode())
            ser.write(passwordbytes)

        if command in {"sign"}:
            hash = prompt("Enter hash: ")
            ser.write(bytes.fromhex(hash))

        # If the command is "restore", send the backup
        elif command == "restore":
            backup = prompt("Enter backup name (filename on the SD Card): ")
            ser.write((backup + '\n').encode())

        while not ser.in_waiting:
            time.sleep(0.1)
        # Read and print the response
        while ser.in_waiting:
            response = ser.readline().decode().strip()
            print(f"Response: {response}")

if __name__ == "__main__":
    main()

