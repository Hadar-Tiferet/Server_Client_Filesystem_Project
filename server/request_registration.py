import request
import connection_utility
import uuid
import struct
import response


class RequestRegistration(request.Request):
    def __init__(self, server_database, net_protocol, client_id, version, code, payload_size):
        if code != net_protocol.request_code["registration"]:
            raise Exception('unrecognized request')
        request.Request.__init__(self, server_database, net_protocol, client_id, version, code, payload_size)
        # for ease of operation tracking
        print("received a registration request from client")

    def process(self, conn_socket):
        if self.payload_size != self.protocol.payload_size["name"]:
            raise Exception('Error: registration request does not contain the correct payload size')
        buffer = connection_utility.receive_exact_bytes(conn_socket, self.protocol.payload_size["name"])
        buffer_format = '<' + self.protocol.payload_format["name"]
        unpacked = struct.unpack(buffer_format, buffer)[0]
        name_check = self.protocol.get_verified_name(unpacked)
        # check if valid name (ascii and null terminated)
        if not name_check[0]:
            # request does not fit the protocol
            raise Exception('Error: registration request contains an invalid client name')
        client_name = name_check[1]
        # the name fits the protocol, check if it does not already exist in the database,
        # and if so register it and return a client id
        if self.database.existing_name(client_name):
            # user is already registered
            return response.ResponseRegistrationFailed(self.protocol, self.database)

        new_uuid = uuid.uuid4().bytes
        while not self.database.add_client(new_uuid, client_name):
            # user was not registered since the uuid is in use, generate a new uuid and try again
            new_uuid = uuid.uuid4().bytes
        # user registration successful
        return response.ResponseRegistrationSuccess(self.protocol, self.database, new_uuid)


if __name__ == "__main__":
    pass
