class Protocol:
    def __init__(self, clients_files_folder='./'):
        self.request_code = {"registration": 1100,
                             "public_key": 1101,
                             "send_file": 1103,
                             "crc_correct": 1104,
                             "crc_retry": 1105,
                             "crc_reject": 1106
                             }
        self.response_code = {"registration_success": 2100,
                              "registration_failed": 2101,
                              "key_exchange": 2102,
                              "file_received": 2103,
                              "crc_verified": 2104
                              }
        self.header_size = {"client_id": 16,
                            "version": 1,
                            "code": 2,
                            "payload_size": 4
                            }
        self.header_format = {"client_id": '16s',
                              "version": 'B',
                              "code": 'H',
                              "payload_size": 'I'
                              }

        self.payload_size = {"name": 255,
                             "client_id": 16,
                             "public_key": 160,
                             "content_size": 4,
                             "file_name": 255,
                             "checksum": 4
                             }
        self.payload_format = {"name": '255s',
                               "client_id": '16s',
                               "public_key": '160s',
                               "content_size": 'I',
                               "file_name": '255s',
                               "checksum": 'I'
                               }

        self.bit_max_value = {8: 255,
                              16: 65535,
                              32: 4294967295
                              }
        self.files_folder = clients_files_folder

    def verify_client_id(self, buffer):
        return len(buffer) == self.header_size['client_id']

    def verify_version(self, buffer):
        correct_type = type(buffer) is int
        correct_value = (buffer >= 0) and (buffer <= self.bit_max_value[8])
        return correct_type and correct_value

    def verify_code(self, buffer):
        correct_type = type(buffer) is int
        correct_value = (buffer >= 0) and (buffer <= self.bit_max_value[16])
        return correct_type and correct_value

    def verify_payload_size(self, buffer):
        correct_type = type(buffer) is int
        correct_value = (buffer >= 0) and (buffer <= self.bit_max_value[32])
        return correct_type and correct_value

    def get_verified_name(self, buffer):
        if (len(buffer) != self.payload_size['name']) or (not buffer.isascii()):
            return False, None
        name = buffer.decode('ascii').split('\x00', 1)[0].strip()
        if (len(name) > 0) and (len(name) < self.payload_size['name']):
            return True, name

        return False, None

    def verify_public_key(self, buffer):
        return len(buffer) == self.payload_size['public_key']

    def verify_content_size(self, buffer):
        correct_type = type(buffer) is int
        correct_value = (buffer >= 0) and (buffer <= self.bit_max_value[32])
        return correct_type and correct_value

    def get_verified_file_name(self, buffer):
        if len(buffer) != self.payload_size['file_name']:
            return False, None
        filename = buffer.decode().split('\x00', 1)[0].strip()
        if (len(filename) > 0) and (len(filename) < self.payload_size['file_name']):
            # filename must be smaller than 255 bytes since it must initially be null terminated
            return True, filename

        return False, None


if __name__ == "__main__":
    pass
