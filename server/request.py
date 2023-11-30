import response


class Request:
    def __init__(self, server_database, net_protocol, client_id, version, code, payload_size):
        self.database = server_database
        self.protocol = net_protocol
        self.client_id = client_id
        self.version = version
        self.code = code
        self.payload_size = payload_size
        if not self.check_header():
            raise Exception('Error: request contains an invalid header')

    def process(self, conn_socket):
        pass

    def check_header(self):
        if not self.protocol.verify_client_id(self.client_id):
            return False
        if not self.protocol.verify_version(self.version):
            return False
        if not self.protocol.verify_code(self.code):
            return False
        if not self.protocol.verify_payload_size(self.payload_size):
            return False
        return True


class RequestUnrecognized(Request):
    def __init__(self, server_database, net_protocol, client_id, version, code, payload_size):
        Request.__init__(self, server_database, net_protocol, client_id, version, code, payload_size)

    def process(self, conn_socket):
        return response.NoResponse(self.protocol, self.database)


if __name__ == "__main__":
    pass
