from web3 import Account, Web3
from eth_account._utils.typed_transactions import TypedTransaction

class ETHtx:
    def __init__(self, nonce, to, value, chainId, publicKey):
        self.dynamic_fee_transaction = {
                            "type": 2,  # optional - can be implicitly determined based on max fee params  # noqa: E501
                            "gas": 21000,
                            "maxFeePerGas": 2000000000,
                            "maxPriorityFeePerGas": 2000000000,
                            "data": "",
                            "nonce": nonce,
                            "to": to,
                            "value": value,
                            "chainId": chainId
                        }
        self.fromAddr = Web3().to_checksum_address("0x"+Web3.keccak(bytes.fromhex(publicKey)).hex()[-40:])
        self.TTX = TypedTransaction.from_dict(self.dynamic_fee_transaction)

    def getTxHash(self):
        print("your TX hash",self.TTX.hash().hex())
        return self.TTX.hash()

    def getSignedTX(self, sig):
        assert len(sig)==130
        R = sig[:64]
        S = sig[64:-2]
        V = sig[-2:]
        assert len(R)==64
        assert len(S)==64
        assert len(V)==2
        tmpTX = dict(self.dynamic_fee_transaction)
        tmpTX["r"] = R
        tmpTX["s"] = S
        print(self.fromAddr,R,S,V)
        print("ID",V)
        tmpTX["v"] = (tmpTX["chainId"]*2) + (35+int(V,16))      
        usTX = TypedTransaction.from_dict(tmpTX)
        ADDR_FROM_TX = Account.recover_transaction(usTX.encode())
        print(tmpTX["v"], ADDR_FROM_TX, self.fromAddr)
        if self.fromAddr == ADDR_FROM_TX:
            return usTX.encode()


    