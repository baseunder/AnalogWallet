from web3 import Account, Web3
from eth_account._utils.typed_transactions import TypedTransaction
import time
from prompt_toolkit import prompt

class ETHtx:
    def __init__(self, publicKey, ser, msgs):
        self.fromAddr = Web3().to_checksum_address(
            "0x"+Web3.keccak(bytes.fromhex(publicKey)).hex()[-40:])

        print("Your ETH address is:", self.fromAddr)
        to = input("Enter the TO address: ")
        amount = Web3.to_wei(prompt("Enter the amount in ether: ", default="0.01"), "ether")
        nonce = int(input("Enter the accounts nonce: "))
        chain = int(prompt("Enter the chain ID (1->Mainnet): ", default="1"))
        maxFeePerGas = Web3.to_wei(prompt("Enter max feePerGas in gwei: ", default="20"), "gwei")
        maxPriorityFee = Web3.to_wei(prompt("Enter max priorityFeePerGas in gwei: ", default="1"), "gwei")
        self.build(nonce, to, amount, chain, maxFeePerGas, maxPriorityFee)
        ser.write(int(3).to_bytes(1, "little"))
        ser.write(bytes.fromhex(self.getTxHash().hex()))
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
        if fis==2:
            while not ser.in_waiting:
                time.sleep(0.05)
            signedTX = self.getSignedTX(ser.read(64).hex())
        if signedTX is None:
            print("someting went wrong with your signature")
        else:
            print("Signed TX, you can broadcast your TX using an online Ethereum transaction broadcast service")
            print("Signed TX", signedTX.hex())

    def build(self, nonce, to, value, chainId, maxGasPrice, maxPriorityFee):
        self.dynamic_fee_transaction = {
                            "type": 2,  # optional - can be implicitly determined based on max fee params  # noqa: E501
                            "gas": 21000,
                            "maxFeePerGas": maxGasPrice,
                            "maxPriorityFeePerGas": maxPriorityFee,
                            "data": "",
                            "nonce": nonce,
                            "to": to,
                            "value": value,
                            "chainId": chainId,
                        }
        self.TTX = TypedTransaction.from_dict(self.dynamic_fee_transaction)

    def getTxHash(self):
        print("your TX hash", self.TTX.hash().hex())
        return self.TTX.hash()

    def getSignedTX(self, sig):
        assert len(sig) == 128
        R = sig[:64]
        S = sig[64:]
        assert len(R) == 64
        assert len(S) == 64
        recoverAddr = []
        # estimate v value by comparing the tx recover address with the public key address
        for z in range(2):
            tmpTX = dict(self.dynamic_fee_transaction)
            tmpTX["r"] = R
            tmpTX["s"] = S
            tmpTX["v"] = z
            usTX = TypedTransaction.from_dict(tmpTX)
            try:
                ADDR_FROM_TX = Account.recover_transaction(usTX.encode())
                if ADDR_FROM_TX == self.fromAddr:
                    recoverAddr = [ADDR_FROM_TX, z, dict(tmpTX), usTX]
                    break
            except Exception as e:
                print(e)
        assert self.fromAddr == recoverAddr[0]
        return TypedTransaction.from_dict(recoverAddr[2]).encode()
