import request
import connection_utility
import struct
from os import remove
import response


class RequestCRCCorrect(request.Request):
    def __init__(self, server_database, net_protocol, client_id, version, code, payload_size):
        if code != net_protocol.request_code["crc_correct"]:
            raise Exception('unrecognized request')
        request.Request.__init__(self, server_database, net_protocol, client_id, version, code, payload_size)
        # for ease of operation tracking
        print("received a crc correct request from client")

    def process(self, conn_socket):
        if self.payload_size != self.protocol.payload_size["client_id"] + self.protocol.payload_size["file_name"]:
            raise Exception('Error: correct crc request does not contain the correct payload size')
        # make sure the client is already registered
        if not self.database.existing_id(self.client_id):
            # client is not registered
            raise Exception('Error: correct crc request arrived for an unregistered client')
        # client made contact with the server - update last seen time
        self.database.update_last_seen(self.client_id)
        # process request payload
        buffer = connection_utility.receive_exact_bytes(
            conn_socket, self.protocol.payload_size["client_id"] + self.protocol.payload_size["file_name"])
        buffer_format = '<' + self.protocol.payload_format["client_id"] + self.protocol.payload_format["file_name"]
        client_id, filename = struct.unpack(buffer_format, buffer)
        # verify that the client id given in the header fits the id in the payload
        if client_id != self.client_id:
            raise Exception('Error: correct crc request contains inconsistent client id')
        filename_check = self.protocol.get_verified_file_name(filename)
        if not filename_check[0]:
            raise Exception('Error: correct crc request contains an invalid payload field')
        # get the filename sent by the client as a string
        clientside_filename = filename_check[1]

        # verify the specified file, if exists
        if not self.database.verify_file_status(self.client_id, clientside_filename, 1):
            # request does not fit the protocol
            raise Exception('Error: correct crc request failed, file does not exist for the client on the server')

        # return success
        return response.ResponseCRCVerified(self.protocol, self.database)


class RequestCRCFailed(request.Request):
    def __init__(self, server_database, net_protocol, client_id, version, code, payload_size):
        if code != net_protocol.request_code["crc_retry"] and code != net_protocol.request_code["crc_reject"]:
            raise Exception('unrecognized request')
        request.Request.__init__(self, server_database, net_protocol, client_id, version, code, payload_size)
        # for ease of operation tracking
        print("received a crc failed request from client")

    def process(self, conn_socket):
        if self.payload_size != self.protocol.payload_size["client_id"] + self.protocol.payload_size["file_name"]:
            raise Exception('Error: failed crc request does not contain the correct payload size')
        # make sure the client is already registered
        if not self.database.existing_id(self.client_id):
            # client is not registered
            raise Exception('Error: failed crc request arrived for an unregistered client')
        # client made contact with the server - update last seen time
        self.database.update_last_seen(self.client_id)
        # process request payload
        buffer = connection_utility.receive_exact_bytes(
            conn_socket, self.protocol.payload_size["client_id"] + self.protocol.payload_size["file_name"])
        buffer_format = '<' + self.protocol.payload_format["client_id"] + self.protocol.payload_format["file_name"]
        client_id, filename = struct.unpack(buffer_format, buffer)
        # verify that the client id given in the header fits the id in the payload
        if client_id != self.client_id:
            raise Exception('Error: failed crc request contains inconsistent client id')
        filename_check = self.protocol.get_verified_file_name(filename)
        if not filename_check[0]:
            raise Exception('Error: failed crc request contains an invalid payload field')
        # get the filename sent by the client as a string
        clientside_filename = filename_check[1]

        serverside_filecheck = self.database.retrieve_filepath(self.client_id, clientside_filename)
        if not serverside_filecheck[0]:
            raise Exception('Error: failed crc request contains failed to locate requested file in the database')
        serverside_filename = serverside_filecheck[1]
        if len(serverside_filename) > 0:
            try:
                remove(serverside_filename)  # remove the file, if already exists
            except OSError:
                pass
        self.database.remove_file(self.client_id, clientside_filename)

        return response.NoResponse(self.protocol, self.database)


if __name__ == "__main__":
    pass
