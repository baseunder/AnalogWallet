import serial
import time
import hashlib
import sys
import os
import ecdsa

from prompt_toolkit import prompt
from serial.tools import list_ports
from pw_check import password_check

from chains.ethereum import ETHtx

class AnalogWallet:
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
        17: "Signing failed",
        18: "Device ID and card status:",
        19: "Firmware hash:",
        20: "Try signing...",
        21: "Device is blocked until reconnect"
    }

    @staticmethod
    def verify(pubkString, hashString, signatureString):
        class dummyHash:
            def __init__(self, hash) -> None:
                self.hash = hash

            def digest(self):
                return self.hash
        pub_key_bytes = bytes.fromhex(pubkString)
        hsh = bytes.fromhex(hashString)
        sig_bytes = bytes.fromhex(signatureString)
        vk = ecdsa.VerifyingKey.from_string(pub_key_bytes, curve=ecdsa.SECP256k1)
        veri = ecdsa.VerifyingKey.verify(vk, sig_bytes, hsh, hashfunc=dummyHash)
        return veri

    @staticmethod
    def getPasswordBytes():
        password = prompt("Enter password/secret: ", is_password=True)
        if not password_check(password):
            return None
        passwordbytes = hashlib.sha256(password.encode()).digest()
        return passwordbytes        
    def waitForDisconnect(self):
        ports = list_ports.comports()
        print("disconnect your wallet now")
        while True:
            newports = list_ports.comports()
            if len(ports)>len(newports):
                return
            time.sleep(0.5)
    def __init__(self, port_name, showCommands=False):
        self.ser = serial.Serial(port_name, 115200)
        self.commands = ['init', 'open', 'erase', 'sign', 'restore', 'rnd', 'getid', 'firmwarehash']
        self.chaincommands = ['ether']
        self.commands += self.chaincommands
        if showCommands:
            print("-".join([" " for _ in range(10)]))
            print("\n".join(self.commands))
            print("-".join([" " for _ in range(10)]))
        self.deviceOpen = False
        self.PUBLIC_KEY = None
        self.HASH_TO_SIGN = None
        self.SIGNATURE = None
        self.PASSWORD = None
        self.HEXHASH = None

    def guide(self):
        command = prompt("Enter command: ")
        if not (command in self.commands):
            if command == "exit":
                print("AnalogWallet session ends now...")
                return
            else:
                print("command", command, "not found")
                return
        if command in self.chaincommands:
            if self.deviceOpen:
                ETHtx(self.PUBLIC_KEY, self.ser, self.msgs)
                return
            else:
                print("Wallet is not open")
                return
        self.run(command)
    
    def write(self, data):
        self.ser.write(data)
        self.ser.flush()
        while self.ser.out_waiting:
            time.sleep(0.01)

    def run(self, command):
        cmIndex = self.commands.index(command).to_bytes(1, "little")
        if command in {"open", "firmwarehash"}:
            if self.PASSWORD is None:
                self.PASSWORD = self.getPasswordBytes()
                if not self.PASSWORD:
                    return
            self.write(cmIndex)
            self.write(self.PASSWORD)
        elif command in {"init"}:
            if self.PASSWORD is None:
                self.PASSWORD = self.getPasswordBytes()
                if not self.PASSWORD:
                    return
            self.write(cmIndex)
            self.write(self.PASSWORD)
            self.write(hashlib.sha256(str(time.time()).encode()).digest())
        elif command == "sign":
            if self.HEXHASH is None:
                self.HEXHASH = prompt("Enter hash [HEX]: ")
                self.HEXHASH = bytes.fromhex(self.HEXHASH)
                if len(self.HEXHASH) != 32:
                    print("hash not valid")
                    return
            self.write(cmIndex)
            self.write(self.HEXHASH)
        elif command == "restore":
            print(
                "The device blocks after the backup is done, you have to replug it to continue.")
            self.write(cmIndex)
        elif command == "erase":
            self.write(cmIndex)
        elif command == "rnd":
            ofname = input("provide a filename for your random data:")
            if os.path.exists(ofname):
                print("this file does already exist...")
                exit()
            output_file = open(ofname, 'wb')
            self.write(cmIndex)
            endsize = int(10000000/256)  # 10MB
            count = 0
            try:
                while True:
                    received_data = self.ser.read(256)
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
        elif command == "getid":
            self.write(cmIndex)
        return command

    def parseResponses(self, command):
        while True:
            while not self.ser.in_waiting:
                time.sleep(0.05)
            fis = int.from_bytes(self.ser.read(), "little")
            if fis in self.msgs.keys():
                print(self.msgs[fis])
                if fis == 1:
                    while not self.ser.in_waiting:
                        time.sleep(0.05)
                    fis = self.ser.read(64)
                    print(fis.hex())
                    self.PUBLIC_KEY = fis.hex()
                    self.deviceOpen = True
                elif fis == 2:
                    while not self.ser.in_waiting:
                        time.sleep(0.05)
                    fis = self.ser.read(64)
                    print(fis.hex())
                    self.SIGNATURE = fis.hex()
                    assert self.verify(self.PUBLIC_KEY, self.HASH_TO_SIGN, self.SIGNATURE)
                    print(
                        "SIGNATURE TEST OK:\nsignature created by the private key is verified with the public key")
                elif fis == 6 or fis == 7:
                    self.waitForDisconnect()
                    exit()
                elif fis == 16:
                    while not self.ser.in_waiting:
                        time.sleep(0.05)
                    fis = self.ser.read(32)
                    print(fis.hex())
                    self.HASH_TO_SIGN = fis.hex()
                elif fis == 14 or fis == 15:
                    while not self.ser.in_waiting:
                        time.sleep(0.05)
                    fis = self.ser.read(8)
                    print(fis)
                elif fis == 18:
                    while not self.ser.in_waiting:
                        time.sleep(0.05)
                    fis = self.ser.read(32)
                    print(fis.hex())
                    fis = self.ser.read(1)
                    print(fis.hex())
                elif fis == 19:
                    while not self.ser.in_waiting:
                        time.sleep(0.05)
                    fis = self.ser.read(32)
                    print(fis.hex())
                elif fis == 20:
                    pass
                elif fis == 21:
                    self.waitForDisconnect()
                else:
                    if command == "init":
                        print("Unplug the device and remove the SD card now, store the backup SD card in a secure place, it is the only possibility to restore your wallet.")
                        print("Write down your password, a restore of the wallet with your SD card is only possible if you have the right password.")
                    break
            else:
                print(fis)

def waitForConnect():
    ports = list_ports.comports()
    print("connect your wallet now")
    while True:
        newports = list_ports.comports()
        if len(ports)<len(newports):
            return list(set(newports).symmetric_difference(set(ports)))[0].device
        time.sleep(0.5)

if __name__ == "__main__":
    if "test" in sys.argv:
        cmds = ["erase", "firmwarehash", "init", "reconnect", "getid", "open", "sign", "reconnect", "erase", "restore", "reconnect", "open"]
        aw = AnalogWallet(waitForConnect())
        aw.PASSWORD = hashlib.sha256("12345".encode()).digest()
        aw.HEXHASH = hashlib.sha256("test".encode()).digest()
        for cmd in cmds:
            if cmd == "reconnect":
                aw.waitForDisconnect()
                aw = AnalogWallet(waitForConnect())
                aw.PASSWORD = hashlib.sha256("12345".encode()).digest()
                aw.HEXHASH = hashlib.sha256("test".encode()).digest()
            else:
                print("send",cmd)
                aw.run(cmd)
                aw.parseResponses(cmd)
    else:
        aw = AnalogWallet(waitForConnect(), True)
        while True:
            cmd = aw.guide()
            aw.parseResponses(cmd)
