from web3 import Account, Web3
from eth_account._utils.typed_transactions import TypedTransaction
from eth_account._utils.legacy_transactions import encode_transaction
import time
from prompt_toolkit import prompt
import ecdsa

class ETHtx:
    def __init__(self, publicKey, ser, msgs):
        self.fromAddr = Web3().to_checksum_address(
            "0x"+Web3.keccak(bytes.fromhex(publicKey)).hex()[-40:])
        print("Your ETH address is:", self.fromAddr)
        if ser:
            to = input("Enter the TO address: ")
            amount = Web3.to_wei(prompt("Enter the amount in ether: ", default="0.01"), "ether")
            nonce = int(input("Enter the accounts nonce: "))
            chain = int(prompt("Enter the chain ID (1->Mainnet): ", default="1"))
            maxFeePerGas = Web3.to_wei(prompt("Enter max feePerGas in gwei: ", default="20"), "gwei")
            maxPriorityFee = Web3.to_wei(prompt("Enter max priorityFeePerGas in gwei: ", default="1"), "gwei")
            self.build(nonce, to, amount, chain, maxFeePerGas, maxPriorityFee)
            ser.write(int(3).to_bytes(1, "little"))
            TXHASH = self.getTxHash().hex()
            ser.write(bytes.fromhex(TXHASH))
            while not ser.in_waiting:
                time.sleep(0.05)
            fis=int.from_bytes(ser.read(),"little")
            print(msgs[fis])
            while not ser.in_waiting:
                time.sleep(0.05)
            fis = ser.read(32)
            print(fis.hex())
            fis=int.from_bytes(ser.read(),"little")
            print(msgs[fis])
            signedTX = None
            SIGNATURE = ser.read(64).hex()
            if fis==2:
                while not ser.in_waiting:
                    time.sleep(0.05)
                signedTX = self.getSignedTX(SIGNATURE)
            if signedTX is None:
                print("someting went wrong with your signature")
            else:
                if self.verify(publicKey, TXHASH, SIGNATURE):
                    print("Signature verified")
                    print("Signed TX, you can broadcast your TX using an online Ethereum transaction broadcast service")
                    print("Signed TX", signedTX.hex())
                else:
                    print("someting went wrong with your signature (ecdsa verification)")

    def verify(self, pubkString, hashString, signatureString):
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

    def build(self, nonce, to, value, chainId, maxGasPrice, maxPriorityFee):
        self.dynamic_fee_transaction = {
                            "type": 2,
                            "gas": 21000,
                            "maxFeePerGas": maxGasPrice,
                            "maxPriorityFeePerGas": maxPriorityFee,
                            "nonce": nonce,
                            "to": to,
                            "value": value,
                            "chainId": chainId,
                        }
        self.TTX = TypedTransaction.from_dict(self.dynamic_fee_transaction)
        print(self.TTX)
        
    def getTxHash(self):
        print("your TX hash", self.TTX.hash().hex())
        return self.TTX.hash()

    def getSignedTX(self, signature):
        assert len(signature) == 128
        R = int(signature[:64],16)
        S = int(signature[64:],16)
        if S > int("0x7fffffffffffffffffffffffffffffff5d576e7357a4501ddfe92f46681b20a0",16):
            #print("S is bigger than the curveOrder/2, invert S...")
            S = int("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141", 16)-S
        dynamic_fee_transaction = dict(self.dynamic_fee_transaction).copy()
        dynamic_fee_transaction["r"] = R
        dynamic_fee_transaction["s"] = S
        for z in range(2):
            dynamic_fee_transaction["v"] = z
            tx = TypedTransaction.from_dict(dynamic_fee_transaction)
            try:
                calcFrom = Account.recover_transaction(tx.encode())
                if calcFrom.lower() == self.fromAddr.lower():
                    print("hash",tx.hash().hex())
                    print(tx.encode().hex())
                    return tx.encode()
            except:
                pass
        return None
