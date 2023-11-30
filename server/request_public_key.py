import request
import connection_utility
import struct
from Crypto.Cipher import PKCS1_OAEP
from Crypto.PublicKey import RSA
from Crypto.Random import get_random_bytes
import response


class RequestPublicKey(request.Request):
    def __init__(self, server_database, net_protocol, client_id, version, code, payload_size):
        if code != net_protocol.request_code["public_key"]:
            raise Exception('unrecognized request')
        request.Request.__init__(self, server_database, net_protocol, client_id, version, code, payload_size)
        # for ease of operation tracking
        print("received a key exchange request from client")

    def process(self, conn_socket):
        if self.payload_size != self.protocol.payload_size["name"] + self.protocol.payload_size["public_key"]:
            raise Exception('Error: key exchange request does not contain the correct payload size')
        # make sure the client is already registered
        if not self.database.existing_id(self.client_id):
            # client is not registered
            raise Exception('Error: key exchange request arrived for an unregistered client')
        # client made contact with the server - update last seen time
        self.database.update_last_seen(self.client_id)
        # process request payload
        buffer = connection_utility.receive_exact_bytes(
            conn_socket, self.protocol.payload_size["name"] + self.protocol.payload_size["public_key"])
        buffer_format = '<' + self.protocol.payload_format["name"] + self.protocol.payload_format["public_key"]
        name, public_key = struct.unpack(buffer_format, buffer)
        name_check = self.protocol.get_verified_name(name)
        # check if valid name (ascii and null terminated)
        if (not name_check[0]) or (not self.protocol.verify_public_key(public_key)):
            # request does not fit the protocol
            raise Exception('Error: key exchange request contains an invalid payload field')
        # get the clients name as a string
        client_name = name_check[1]

        # received a name and a public key from the client - generate an AES key
        aes_key = get_random_bytes(16)  # the key to be used for AES in CBC mode
        # encrypt the AES key with the client's public key
        client_public_key = RSA.import_key(public_key)
        cipher_rsa = PKCS1_OAEP.new(client_public_key)
        encrypted_aes_key = cipher_rsa.encrypt(aes_key)
        # update the client details in the server database
        self.database.update_client(self.client_id, client_name, public_key, aes_key)
        # build a response to the client
        return response.ResponseKeyExchange(self.protocol, self.database, self.client_id, encrypted_aes_key)


if __name__ == "__main__":
    pass
