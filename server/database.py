import sqlite3


class DB:
    def __init__(self, database_name="server.db"):
        self.exported = False
        self.con = sqlite3.connect(database_name, check_same_thread=False)
        self.con.text_factory = bytes
        self.con.executescript("""
                            CREATE TABLE IF NOT EXISTS clients (ID BLOB NOT NULL PRIMARY KEY, Name varchar(255),
                            PublicKey BLOB, LastSeen varchar(255), AESKey BLOB);
                            CREATE TABLE IF NOT EXISTS files (ID BLOB NOT NULL, filename TEXT,
                            pathname TEXT PRIMARY KEY, Verified int);
                            """)

    def export_data(self):
        """ Export the data from both tables, only once during program initialization """
        if self.exported is True:
            return None, None
        # prevent future access during the current run
        self.exported = True
        with self.con:
            request1 = "SELECT * FROM clients"
            request2 = "SELECT * FROM files"
            clients_table = self.con.execute(request1).fetchall()
            files_table = self.con.execute(request2).fetchall()
            return clients_table, files_table

    def existing_name(self, client_name):
        """ Given a client's name, check for the existence of such a client in the database """
        with self.con:
            request = "SELECT ID FROM clients WHERE Name = ?"
            result = self.con.execute(request, (client_name + '\0', )).fetchone()
            if result is None:
                return False
            else:
                return True

    def existing_id(self, client_id):
        """ Given a client's ID, check for the existence of such a client in the database """
        with self.con:
            request = "SELECT ID FROM clients WHERE ID = ?"
            result = self.con.execute(request, (client_id, )).fetchone()
            if result is None:
                return False
            else:
                return True

    def add_client(self, client_id, client_name):
        """ Attempt to add a new client to the database, if the client id is unique, and return success indication """
        try:
            with self.con:
                request = "INSERT INTO clients VALUES (?, ?, ?, datetime('now', 'localtime'), ?)"
                self.con.execute(request, (client_id, client_name + '\0', "", ""))
                return True
        except sqlite3.IntegrityError:
            # can't add a new client with an already existing ID
            print("attempt to add a new client with an existing ID")
            return False
        except Exception as e:
            # client addition failed
            print(e)
            return False

    def update_last_seen(self, client_id):
        """ update the last seen time of a given client to the current local time """
        with self.con:
            request = "UPDATE clients SET LastSeen = datetime('now', 'localtime') WHERE ID = ?"
            self.con.execute(request, (client_id, ))

    def update_client(self, client_id, client_name, public_key, aes_key):
        """ update client information in the database based on ID and name """
        with self.con:
            request = "UPDATE clients SET PublicKey = ?, AESKey = ? WHERE ID = ? AND Name = ?"
            self.con.execute(request, (public_key, aes_key, client_id, client_name + '\0'))

    def retrieve_key(self, client_id):
        """ retrieve a client's connection aes key based on the client's ID """
        with self.con:
            request = "SELECT AESKey FROM clients WHERE ID = ?"
            result = self.con.execute(request, (client_id, )).fetchone()[0]
            return result

    def add_available_filepath(self, client_id, client_filename, server_pathname):
        """ Attempt to add a new file by the client to the database, if the serverside pathname is unique,
        and return success indication """
        try:
            with self.con:
                request = "INSERT INTO files VALUES (?, ?, ?, ?)"
                self.con.execute(request, (client_id, client_filename + '\0', server_pathname + '\0', 0))
                return True
        except sqlite3.IntegrityError:
            # can't add a new client with an already existing ID
            print("attempt to add a new client file with an filepath already in use")
            return False
        except Exception as e:
            # client addition failed
            print(e)
            return False

    def verify_file_status(self, client_id, client_filename, status):
        """ update the verified status of a given file registered to the client """
        with self.con:
            request = "SELECT Verified FROM files WHERE ID = ? AND filename = ?"
            result = self.con.execute(request, (client_id, client_filename + '\0')).fetchone()[0]
            if result != 1 - status:
                # the requested file is either verified or is not stored on the server
                return False
            request = "UPDATE files SET Verified = ? WHERE ID = ? AND filename = ?"
            self.con.execute(request, (status, client_id, client_filename + '\0'))
            return True

    def retrieve_filepath(self, client_id, client_filename):
        """ retrieve the server-side file path for a file given the client's ID and client-side file name if exists"""
        with self.con:
            request = "SELECT pathname FROM files WHERE ID = ? AND filename = ?"
            result = self.con.execute(request, (client_id, client_filename + '\0')).fetchone()
            if result is None:
                return False, None
            return True, result[0].decode().strip('\0')

    def remove_file(self, client_id, client_filename):
        """ remove a listing for a file stored on the server based on the client's ID and client-side file name """
        with self.con:
            request = "DELETE FROM files WHERE ID = ? AND filename = ?"
            self.con.execute(request, (client_id, client_filename + '\0'))


if __name__ == "__main__":
    pass
