import request
import connection_utility
import struct
from Crypto.Cipher import AES
from Crypto.Util.Padding import unpad
from Crypto.Random import random
from os import remove
import crc
import response


class RequestSendFile(request.Request):
    def __init__(self, server_database, net_protocol, client_id, version, code, payload_size):
        if code != net_protocol.request_code["send_file"]:
            raise Exception('unrecognized request')
        request.Request.__init__(self, server_database, net_protocol, client_id, version, code, payload_size)
        # for ease of operation tracking
        print("received a send file request from client")

    def process(self, conn_socket):
        # make sure the client is already registered
        if not self.database.existing_id(self.client_id):
            # client is not registered
            raise Exception('Error: send file request arrived for an unregistered client')
        # client made contact with the server - update last seen time
        self.database.update_last_seen(self.client_id)
        # process request payload
        buffer = connection_utility.receive_exact_bytes(
            conn_socket, self.protocol.payload_size["client_id"] + self.protocol.payload_size["content_size"] +
            self.protocol.payload_size["file_name"])
        buffer_format = '<' + self.protocol.payload_format["client_id"] + self.protocol.payload_format["content_size"] \
                        + self.protocol.payload_format["file_name"]
        client_id, content_size, filename = struct.unpack(buffer_format, buffer)
        # verify that the client id given in the header fits the id in the payload
        if client_id != self.client_id:
            raise Exception('Error: send file request contains inconsistent client id')
        filename_check = self.protocol.get_verified_file_name(filename)
        if (not self.protocol.verify_content_size(content_size)) or (not filename_check[0]):
            raise Exception('Error: send file request contains an invalid payload field')
        # get the filename sent by the client as a string
        clientside_filename = filename_check[1]
        if self.payload_size != self.protocol.payload_size["client_id"] + self.protocol.payload_size["content_size"] \
                + self.protocol.payload_size["file_name"] + content_size:
            raise Exception('Error: send file request does not contain the correct payload size')

        # create a serverside path for the file to be stored at
        # check if the requested file is already saved on the server
        serverside_filecheck = self.database.retrieve_filepath(self.client_id, clientside_filename)
        if serverside_filecheck[0]:
            # file is currently saved on the server
            serverside_filename = serverside_filecheck[1]
            self.database.verify_file_status(self.client_id, clientside_filename, 0)
        else:
            # file is currently not saved on the server, generate a unique filepath for it and save to the database
            free_chars = self.protocol.payload_size['file_name'] - 1 - len(self.protocol.files_folder)
            serverside_filename = self.protocol.files_folder + self.get_random_filename(free_chars)
            while not self.database.add_available_filepath(self.client_id, clientside_filename, serverside_filename):
                serverside_filename = self.protocol.files_folder + self.get_random_filename(free_chars)

        # file is now registered in the database
        # for ease of operation tracking
        print("attempt to receive and decipher a file from client")
        if not self.receive_and_decipher_file(conn_socket, serverside_filename, content_size):
            # request does not fit the protocol
            try:
                remove(serverside_filename)  # remove the file, if already exists
            except OSError:
                pass
            self.database.remove_file(self.client_id, clientside_filename)  # remove the file from the database
            raise Exception('Error: send file request encountered an error writing data to file')
        # for ease of operation tracking
        print("received and decipher a file from client, calculating crc value for file")
        checksum = crc.crc32().calculate_crc(serverside_filename)

        return response.ResponseFileReceived(
            self.protocol, self.database, self.client_id, content_size, clientside_filename, checksum)

    def receive_and_decipher_file(self, conn_socket, serverside_filename, content_size):
        try:
            aes_iv = bytearray(16)
            aes_key = self.database.retrieve_key(self.client_id)
            cipher = AES.new(aes_key, AES.MODE_CBC, aes_iv)

            with open(serverside_filename, 'wb') as file:
                buffer = connection_utility.receive_exact_bytes(conn_socket, content_size)
                file.write(unpad(cipher.decrypt(buffer), AES.block_size))
            return True
        except Exception as e:
            # encountered an issue while writing or decrypting content to file
            print(e)
            return False

    def get_random_filename(self, length):
        charset = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789'
        string = ''
        for i in range(length):
            string += random.choice(charset)
        return string


if __name__ == "__main__":
    pass
