
def receive_exact_bytes(connection, byte_amount):
    buffer = connection.recv(byte_amount)
    received = len(buffer)
    while received < byte_amount:
        buf = connection.recv(byte_amount - received)
        if len(buf) == 0:
            raise Exception('connection has been closed prematurely')
        buffer += buf
        received += len(buf)
    # exactly byte_amount bytes were received
    return buffer


if __name__ == "__main__":
    pass
