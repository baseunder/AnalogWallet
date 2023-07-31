import serial
import time
import hashlib
import sys
import os
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
        port_completer = WordCompleter(port_names, ignore_case=True, match_middle=True)
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
            if command == "exit":
                print("AnalogWallet session ends now...")
                break
            else:
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
        elif command == "sign":
            hash = prompt("Enter hash [HEX]: ")
            hashB = bytes.fromhex(hash)
            if len(hashB) != 32:
                print("hash not valid")
                continue
            ser.write(cmIndex)
            ser.write(hashB)
        elif command == "restore":
            print("The device blocks after the backup is done, you have to replug it to continue.")
            backup = prompt("Enter backup name (filename on the SD Card): ")
            ser.write(cmIndex)
            ser.write(bytes([int(backup[i:i+2], 16) for i in range(0,8,2)]))
        elif command == "erase":
            ser.write(cmIndex)
        elif command == "rnd":
            ofname = input("provide a filename for your random data:")
            if os.path.exists(ofname):
                print("this file does already exist...")
                exit()
            output_file = open(ofname, 'wb')
            ser.write(cmIndex)
            endsize = int(10000000/256) # 10MB
            count = 0
            try:
                while True:
                    received_data = ser.read(256)  # Specify the number of bytes to read
                    if len(received_data)<256:
                        raise KeyboardInterrupt
                    output_file.write(received_data)
                    print("size:", count*256, "sample:",int(received_data.hex()[:2],16),int(received_data.hex()[2:4],16),int(received_data.hex()[4:6],16))
                    count += 1
                    if count > endsize:
                        raise KeyboardInterrupt
            except KeyboardInterrupt:
                output_file.close()
            print("your random data is now generated:",output_file)
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
                if command == "init":
                    print("Unplug the device and remove the SD card now, store the backup SD card in a secure place, it is the only possibility to restore your wallet.")
                    print("Write down your password, a restore of the wallet with your SD card is only possible if you have the right password.")
                break

if __name__ == "__main__":
    main()

