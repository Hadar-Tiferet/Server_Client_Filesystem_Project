import struct


class Response:
    def __init__(self, net_protocol, server_database, version=3):
        self.protocol = net_protocol
        self.database = server_database
        self.version = version

    def execute(self, conn_socket):
        pass


class ResponseRegistrationSuccess(Response):
    def __init__(self, net_protocol, server_database, client_id):
        Response.__init__(self, net_protocol, server_database)
        self.code = self.protocol.response_code["registration_success"]
        self.client_id = client_id

    def execute(self, conn_socket):
        payload_size = self.protocol.payload_size["client_id"]
        response_format = '<' + self.protocol.header_format["version"] + \
            self.protocol.header_format["code"] + self.protocol.header_format["payload_size"] + \
            self.protocol.payload_format["client_id"]

        conn_socket.send(struct.pack(response_format, self.version, self.code, payload_size, self.client_id))
        # for ease of operation tracking
        print("sent a registration success response to client")


class ResponseRegistrationFailed(Response):
    def __init__(self, net_protocol, server_database):
        Response.__init__(self, net_protocol, server_database)
        self.code = self.protocol.response_code["registration_failed"]

    def execute(self, conn_socket):
        payload_size = 0
        response_format = '<' + self.protocol.header_format["version"] + \
            self.protocol.header_format["code"] + self.protocol.header_format["payload_size"]

        conn_socket.send(struct.pack(response_format, self.version, self.code, payload_size))
        # for ease of operation tracking
        print("sent a registration failed response to client")


class ResponseKeyExchange(Response):
    def __init__(self, net_protocol, server_database, client_id, encrypted_key):
        Response.__init__(self, net_protocol, server_database)
        self.code = self.protocol.response_code["key_exchange"]
        self.client_id = client_id
        self.encrypted_key = encrypted_key

    def execute(self, conn_socket):
        encrypted_key_size = len(self.encrypted_key)
        encrypted_key_format = str(encrypted_key_size) + 's'
        payload_size = self.protocol.payload_size["client_id"] + encrypted_key_size
        response_format = '<' + self.protocol.header_format["version"] + \
            self.protocol.header_format["code"] + self.protocol.header_format["payload_size"] + \
            self.protocol.payload_format["client_id"] + encrypted_key_format

        conn_socket.send(struct.pack(
            response_format, self.version, self.code, payload_size, self.client_id, self.encrypted_key))
        # for ease of operation tracking
        print("sent a key exchange response to client")


class ResponseFileReceived(Response):
    def __init__(self, net_protocol, server_database, client_id, content_size, clientside_filename, checksum):
        Response.__init__(self, net_protocol, server_database)
        self.code = self.protocol.response_code["file_received"]
        self.client_id = client_id
        self.content_size = content_size
        self.filename = clientside_filename.encode()
        self.checksum = checksum

    def execute(self, conn_socket):
        payload_size = self.protocol.payload_size["client_id"] + self.protocol.payload_size["content_size"] + \
            self.protocol.payload_size["file_name"] + self.protocol.payload_size["checksum"]
        response_format = '<' + self.protocol.header_format["version"] + \
            self.protocol.header_format["code"] + self.protocol.header_format["payload_size"] + \
            self.protocol.payload_format["client_id"] + self.protocol.payload_format["content_size"] + \
            self.protocol.payload_format["file_name"] + self.protocol.payload_format["checksum"]

        conn_socket.send(struct.pack(
            response_format, self.version, self.code, payload_size, self.client_id, self.content_size,
            self.filename, self.checksum))
        # for ease of operation tracking
        print("sent a file received response to client")


class ResponseCRCVerified(Response):
    def __init__(self, net_protocol, server_database):
        Response.__init__(self, net_protocol, server_database)
        self.code = self.protocol.response_code["crc_verified"]
        """
        I believe it would make more sense if the response for CRC verified contained the clients id and filename
        but that goes against the protocol as is so it's kept as a comment here
        """
        # self.client_id = client_id
        # self.filename = filename

    def execute(self, conn_socket):
        payload_size = 0
        response_format = '<' + self.protocol.header_format["version"] + \
                          self.protocol.header_format["code"] + self.protocol.header_format["payload_size"]
        conn_socket.send(struct.pack(
            response_format, self.version, self.code, payload_size))
        """
        payload_size = self.protocol.payload_size["client_id"] + self.protocol.payload_size["file_name"]
        response_format = '<' + self.protocol.header_format["version"] + \
            self.protocol.header_format["code"] + self.protocol.header_format["payload_size"] + \
            self.protocol.payload_format["client_id"] + self.protocol.payload_format["file_name"]
        conn_socket.send(struct.pack(
            response_format, self.version, self.code, payload_size, self.client_id, self.filename))
        """
        # for ease of operation tracking
        print("sent a crc verified response to client")


class NoResponse(Response):
    def __init__(self, net_protocol, server_database):
        Response.__init__(self, net_protocol, server_database)

    def execute(self, conn_socket):
        # for ease of operation tracking
        print("sent no response to client")
        return


if __name__ == "__main__":
    pass
