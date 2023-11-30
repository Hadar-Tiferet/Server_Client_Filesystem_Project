import database
from datetime import datetime
from threading import RLock


class ClientData:
    def __init__(self, client_name, public_key=None, last_seen=None, aes_key=None):
        self.client_name = client_name
        self.public_key = public_key
        self.last_seen = last_seen
        self.aes_key = aes_key
        self.files = dict()
        self.client_lock = RLock()  # prevent access to each specific client by more than one thread at a time

    def add_file(self, clientside_filepath, serverside_filepath, verified_status=0):
        """ add the given file alongside its data into the database, overwriting existing file if exists """
        with self.client_lock:
            self.files[clientside_filepath] = [serverside_filepath, verified_status]

    def update_last_seen(self):
        with self.client_lock:
            self.last_seen = datetime.now()

    def update_client(self, client_name, public_key, aes_key):
        with self.client_lock:
            if self.client_name == client_name:
                self.public_key = public_key
                self.aes_key = aes_key

    def get_aes_key(self):
        with self.client_lock:
            return self.aes_key

    def get_serverside_filepath(self, clientside_filepath):
        with self.client_lock:
            if clientside_filepath not in self.files:
                return None
            return self.files[clientside_filepath][0]

    def update_file_verified_status(self, clientside_filepath, verified_status=0):
        with self.client_lock:
            if clientside_filepath not in self.files:
                return False
            if self.files[clientside_filepath][1] != 1 - verified_status:
                return False
            self.files[clientside_filepath][1] = verified_status
            return True

    def get_removed_filepath(self, clientside_filepath):
        with self.client_lock:
            return self.files.pop(clientside_filepath, None)


class ClientInfo:
    def __init__(self, database_name='server.db'):
        self.database = database.DB(database_name)
        # load database info into memory
        self.clients = dict()
        self.files = set()
        self.clients_lock = RLock()  # prevent addition of more than one client at a time
        self.files_lock = RLock()  # prevent addition of more than one file at a time

        clients_table, files_table = self.database.export_data()
        for entry in clients_table:
            self.clients[entry[0]] = ClientData(entry[1].decode().strip('\0'), entry[2], entry[3].decode(), entry[4])
        for entry in files_table:
            self.clients[entry[0]].add_file(entry[1].decode().strip('\0'), entry[2].decode().strip('\0'), entry[3])
            self.files.add(entry[2].decode().strip('\0'))

    def existing_name(self, client_name):
        """ given a client's name, check for the existence of such a client in the server memory """
        for client in self.clients:
            if self.clients[client].client_name == client_name:
                return True
        return False

    def existing_id(self, client_id):
        """ given a client's ID, check for the existence of such a client in the server memory """
        return client_id in self.clients

    def add_client(self, client_id, client_name):
        """ attempt to add a new client to server's memory and update the database, if the client id is unique,
        and return success indication """
        with self.clients_lock:
            if self.existing_id(client_id):
                # a client with the given client id already exists
                return False
            self.clients[client_id] = ClientData(client_name, last_seen=datetime.now())
            # update the database
            self.database.add_client(client_id, client_name)
            return True

    def update_last_seen(self, client_id):
        """ update the last seen time of a given client to the current local time, in memory and in the database """
        self.clients[client_id].update_last_seen()
        self.database.update_last_seen(client_id)

    def update_client(self, client_id, client_name, public_key, aes_key):
        """ update client information in server memory and in the database based on ID and name """
        self.clients[client_id].update_client(client_name, public_key, aes_key)
        self.database.update_client(client_id, client_name, public_key, aes_key)

    def retrieve_key(self, client_id):
        """ retrieve a client's connection aes key based on the client's ID in memory """
        return self.clients[client_id].get_aes_key()

    def add_available_filepath(self, client_id, client_filename, server_pathname):
        """ attempt to add a new file by the client to the servers memory and database, if the serverside
        pathname is unique. return success indication"""
        with self.files_lock:
            if server_pathname in self.files:
                return False
            # server_pathname is not in use by an existing file on the server
            self.clients[client_id].add_file(client_filename, server_pathname)
            self.files.add(server_pathname)
            self.database.add_available_filepath(client_id, client_filename, server_pathname)
            return True

    def verify_file_status(self, client_id, client_filename, status):
        """ update the verified status of a given file registered to the client in the servers memory and database """
        if not self.clients[client_id].update_file_verified_status(client_filename, status):
            return False
        self.database.verify_file_status(client_id, client_filename, status)
        return True

    def retrieve_filepath(self, client_id, client_filename):
        """ retrieve the server-side file path for a file given the client's ID and client-side file name if exists """
        status = True
        result = self.clients[client_id].get_serverside_filepath(client_filename)
        if result is None:
            status = False
        return status, result

    def remove_file(self, client_id, client_filename):
        """ remove a file from the servers memory and the database, based on the client's ID and client-side filename"""
        result = self.clients[client_id].get_removed_filepath(client_filename)
        if result is not None:
            self.files.remove(result)
        self.database.remove_file(client_id, client_filename)


if __name__ == "__main__":
    pass
