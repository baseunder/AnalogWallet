import serial
import time
import hashlib
import sys
import os
import ecdsa

from prompt_toolkit import prompt
from prompt_toolkit.completion import WordCompleter
from serial.tools import list_ports
from pw_check import password_check

from chains.ethereum import ETHtx

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
    15: "Set filename",
    16: "This may take up to 30 seconds, please be patient... Hash to sign:",
    17: "Signing failed"
}

PUBLIC_KEY = None
HASH_TO_SIGN = None
SIGNATURE = None


class dummyHash:
    def __init__(self, hash) -> None:
        self.hash = hash

    def digest(self):
        return self.hash


def verify(pubkString, hashString, signatureString):
    pub_key_bytes = bytes.fromhex(pubkString)
    hsh = bytes.fromhex(hashString)
    sig_bytes = bytes.fromhex(signatureString)
    vk = ecdsa.VerifyingKey.from_string(pub_key_bytes, curve=ecdsa.SECP256k1)
    veri = ecdsa.VerifyingKey.verify(vk, sig_bytes, hsh, hashfunc=dummyHash)
    return veri


def getPasswordBytes():
    password = prompt("Enter password: ", is_password=True)
    if not password_check(password):
        return None
    passwordbytes = hashlib.sha256(password.encode()).digest()
    return passwordbytes


def main():
    port_name = None
    if len(sys.argv) > 1:
        port_name = sys.argv[1]
    else:
        ports = list_ports.comports()
        port_names = [port.device for port in ports][::-1]
        print("-".join([" " for _ in range(10)]))
        print("\n".join(port_names))
        print("-".join([" " for _ in range(10)]))
        port_completer = WordCompleter(
            port_names, ignore_case=True, match_middle=True)
        port_name = prompt("Enter port name: ", completer=port_completer)

    ser = serial.Serial(port_name, 115200)
    commands = ['init', 'open', 'erase', 'sign', 'restore', 'rnd']
    chaincommands = ['sendether']
    print("-".join([" " for _ in range(10)]))
    print("\n".join(commands))
    print("-".join([" " for _ in range(10)]))
    command_completer = WordCompleter(commands)
    deviceOpen = False
    while True:
        command = prompt("Enter command: ", completer=command_completer)
        if not (command in commands):
            if command in chaincommands:
                if deviceOpen:
                    ETHtx(PUBLIC_KEY, ser, msgs)
                else:
                    print("Wallet is not open")
                break
            else:
                if command == "exit":
                    print("AnalogWallet session ends now...")
                    break
                else:
                    print("command", command, "not found")
                    continue
        cmIndex = commands.index(command).to_bytes(1, "little")
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
            print(
                "The device blocks after the backup is done, you have to replug it to continue.")
            backup = prompt("Enter backup name (filename on the SD Card): ")
            ser.write(cmIndex)
            ser.write(bytes([int(backup[i:i+2], 16) for i in range(0, 8, 2)]))
        elif command == "erase":
            ser.write(cmIndex)
        elif command == "rnd":
            ofname = input("provide a filename for your random data:")
            if os.path.exists(ofname):
                print("this file does already exist...")
                exit()
            output_file = open(ofname, 'wb')
            ser.write(cmIndex)
            endsize = int(10000000/256)  # 10MB
            count = 0
            try:
                while True:
                    # Specify the number of bytes to read
                    received_data = ser.read(256)
                    if len(received_data) < 256:
                        raise KeyboardInterrupt
                    output_file.write(received_data)
                    print("size:", count*256, "sample:", int(received_data.hex()[:2], 16), int(
                        received_data.hex()[2:4], 16), int(received_data.hex()[4:6], 16))
                    count += 1
                    if count > endsize:
                        raise KeyboardInterrupt
            except KeyboardInterrupt:
                output_file.close()
            print("your random data is now generated:", output_file)
        while True:
            while not ser.in_waiting:
                time.sleep(0.05)
            fis = int.from_bytes(ser.read(), "little")
            print(msgs[fis])
            if fis == 1:
                while not ser.in_waiting:
                    time.sleep(0.05)
                fis = ser.read(64)
                print(fis.hex())
                PUBLIC_KEY = fis.hex()
                deviceOpen = True
            elif fis == 2:
                while not ser.in_waiting:
                    time.sleep(0.05)
                fis = ser.read(64)
                print(fis.hex())
                SIGNATURE = fis.hex()
                assert verify(PUBLIC_KEY, HASH_TO_SIGN, SIGNATURE)
                print(
                    "SIGNATURE TEST OK:\nsignature created by the private key is verified with the public key")
            elif fis == 16:
                while not ser.in_waiting:
                    time.sleep(0.05)
                fis = ser.read(32)
                print(fis.hex())
                HASH_TO_SIGN = fis.hex()
            elif fis == 14 or fis == 15:
                while not ser.in_waiting:
                    time.sleep(0.05)
                fis = ser.read(8)
                print(fis)
            else:
                if command == "init":
                    print("Unplug the device and remove the SD card now, store the backup SD card in a secure place, it is the only possibility to restore your wallet.")
                    print(
                        "Write down your password, a restore of the wallet with your SD card is only possible if you have the right password.")
                break


if __name__ == "__main__":
    main()
