import serial
import time
import hashlib
import sys
from prompt_toolkit import prompt
from prompt_toolkit.completion import WordCompleter
from serial.tools import list_ports
from pw_check import password_check

msgs = {
        0: "DONE", 
        1: "Public key:", 
        2: "Signature (device blocks after each hash sign action):", 
        3: "Backup OK", 
        4: "Init done", 
        5: "Restore OK", 
        6: "Remove SD card", 
        7: "Insert SD card",
        8: "Device already initialized, perform erase first if you need reinitialization",
        9: "SD card file write ERROR",
        10: "SD card file read ERROR",
        11: "Backup check ERROR",
        12: "Device not initialized",
        13: "SD card file does not exist",
        14: "SD card backup file created",
        15: "Set filename"
        }

def getPasswordBytes():
    password = prompt("Enter password: ", is_password=True)
    if not password_check(password):
        return None
    passwordbytes = hashlib.sha256(password.encode()).digest()
    return passwordbytes
def main():
    port_name = None
    if len(sys.argv)>1:
        port_name = sys.argv[1]
    else:
        # List all available serial ports and let the user select one
        ports = list_ports.comports()
        port_names = [port.device for port in ports][::-1]
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
            passwordbytes = getPasswordBytes()
            if not passwordbytes:
                continue
            ser.write(cmIndex)
            ser.write(passwordbytes)
        elif command in {"sign"}:
            hash = prompt("Enter hash [HEX]: ")
            hashB = bytes.fromhex(hash)
            if len(hashB) != 32:
                print("hash not valid")
                continue
            ser.write(cmIndex)
            ser.write(hashB)
        elif command == "restore":
            backup = prompt("Enter backup name (filename on the SD Card): ")
            ser.write(cmIndex)
            ser.write(bytes([int(backup[i:i+2], 16) for i in range(0,8,2)]))
        elif command == "erase":
            ser.write(cmIndex)
        else:
            ser.write(cmIndex)
        while True:
            while not ser.in_waiting:
                time.sleep(0.05)
            fis=int.from_bytes(ser.read(),"little")
            print(msgs[fis])
            if fis==1 or fis==2:
                while not ser.in_waiting:
                    time.sleep(0.05)
                fis = ser.read(64)
                print(fis.hex())
            elif fis==14 or fis==15:
                while not ser.in_waiting:
                    time.sleep(0.05)
                fis = ser.read(8)
                print(fis)
            else:
                break

if __name__ == "__main__":
    main()

