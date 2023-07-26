import serial
import time
import hashlib
import sys
from prompt_toolkit import prompt
from prompt_toolkit.completion import WordCompleter
from serial.tools import list_ports
from pw_check import password_check

def main():
    port_name = None
    if len(sys.argv)>1:
        port_name = sys.argv[1]
    else:
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
    commands = ['init', 'open', 'erase', 'sign', 'restore', 'rnd']
    print("-".join([" " for _ in range(10)]))
    print("\n".join(commands))
    print("-".join([" " for _ in range(10)]))
    command_completer = WordCompleter(commands)

    while True:
        command = prompt("Enter command: ", completer=command_completer)
        if not (command in commands):
            print("command",command,"not found")
            continue
        cmIndex = commands.index(command).to_bytes(1, "little")
        # If the command requires a second part (like a password or hash), send it
        if command in {"init", "open"}:
            password = prompt("Enter password: ", is_password=True)
            if not password_check(password):
                continue
            passwordbytes = hashlib.sha256(password.encode()).digest()
            ser.write(cmIndex)
            ser.write(passwordbytes)
            while not ser.in_waiting:
                time.sleep(0.05)
            fis = ser.read_all()
            if len(fis)==64:
                print(fis.hex())
                while not ser.in_waiting:
                    time.sleep(0.05)
                fis = ser.read_all()
                print(fis.hex())
            elif len(fis)==1:
                print(fis[0])
        elif command in {"sign"}:
            hash = prompt("Enter hash [HEX]: ")
            hashB = bytes.fromhex(hash)
            if len(hashB) != 32:
                print("hash not valid")
                continue
            ser.write(cmIndex)
            ser.write(hashB)
            while not ser.in_waiting:
                time.sleep(0.05)
            fis = ser.read_all()
            if len(fis)==64:
                print(fis.hex())
            print("the wallet is now closed until you reconnect it")
        elif command == "restore":
            backup = prompt("Enter backup name (filename on the SD Card): ")
            ser.write(cmIndex)
            ser.write((backup + '\n').encode())
            while not ser.in_waiting:
                time.sleep(0.05)
            fis = ser.read()
        elif command == "erase":
            ser.write(cmIndex)
            print(int.from_bytes(ser.read(),"little"))
        else:
            ser.write(cmIndex)
if __name__ == "__main__":
    main()

