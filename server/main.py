import socket
import struct
import protocol
import connection_utility
import request
import request_registration
import request_public_key
import request_send_file
import request_crc
from client_info import ClientInfo
from threading import Thread
from os import makedirs, path


# find out the specific request and process it
def service_client(conn, server_database, files_folder, connection_info):
    try:
        print('connection established with: ' + connection_info)
        pro = protocol.Protocol(clients_files_folder=files_folder)
        size = 0
        for field in pro.header_size:
            size += pro.header_size[field]
        buffer = connection_utility.receive_exact_bytes(conn, size)
        # requested in little endian format with no padding
        header_format = '<'
        for field in pro.header_format:
            header_format += pro.header_format[field]
        client_id, version, code, payload_size = struct.unpack(header_format, buffer)

        # dict containing possible requests and their corresponding class
        requests = {pro.request_code["registration"]: request_registration.RequestRegistration,
                    pro.request_code["public_key"]: request_public_key.RequestPublicKey,
                    pro.request_code["send_file"]: request_send_file.RequestSendFile,
                    pro.request_code["crc_correct"]: request_crc.RequestCRCCorrect,
                    pro.request_code["crc_retry"]: request_crc.RequestCRCFailed,
                    pro.request_code["crc_reject"]: request_crc.RequestCRCFailed
                    }
        if code in requests:
            client_request = requests[code](
                server_database, pro, client_id, version, code, payload_size)
        else:
            # unrecognized request code
            client_request = request.RequestUnrecognized(
                server_database, pro, client_id, version, code, payload_size)
        # process the specific request sent by the user
        response = client_request.process(conn)

        response.execute(conn)
        print('communication ended successfully with: ' + connection_info)
        conn.shutdown(1)
        conn.close()

    except Exception as e:
        # connection has been terminated
        conn.shutdown(1)
        conn.close()
        print("connection with: " + connection_info + " has encountered an error:")
        print(e)
        print("and thus terminated")


def read_server_port(info_file, default_port=1234):
    try:
        with open(info_file, 'r') as file:
            content = file.readline()
            port = int(content.strip())
            if port < 0 or port > 65535:
                raise Exception("Invalid port number read from file")
            return True, port
    except Exception as e:
        # encountered an issue while reading from file, continue with the default port number
        print(e)
        return False, default_port


def main():
    db = ClientInfo('server.db')
    sock = socket.socket()
    host = 'localhost'
    server_info = 'port.info'
    backup_folder = './client_files/'
    # create a folder to hold files sent by clients, unless already exists
    makedirs(path.dirname(backup_folder), exist_ok=True)

    port_detail = read_server_port(server_info)
    if not port_detail[0]:
        print("failed to read a legitimate port for the server from: " + server_info + " , will use default port 1234")

    try:
        sock.bind((host, port_detail[1]))
    except socket.error as e:
        print(str(e))

    print('Server is listening for clients')
    sock.listen(100)

    while True:
        client_socket, address = sock.accept()
        connection_info = str(address[0]) + ':' + str(address[1])
        new_client = Thread(target=service_client, args=(client_socket, db, backup_folder, connection_info))
        new_client.start()


if __name__ == "__main__":
    main()
