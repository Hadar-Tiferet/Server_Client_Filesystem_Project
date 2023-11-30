#include <fstream>
#include <iostream>

#include "Client.h"

bool readServerInfo(std::string server_info, std::string& serverIP,
	std::string& serverPort, std::string& client_name, std::string& filename) {
	try {
		// open the server info file and attempt to read client basic details
		std::ifstream file(server_info, std::ios::in);
		try {
			// variables for items read from file
			std::string connection_details;
			std::getline(file, connection_details);
			// attempt to seperate server ip and port number
			size_t end = connection_details.find(":");
			if (end == -1) {
				// first line of file does not contain the required information
				return false;
			}
			serverIP = connection_details.substr(0, end);
			serverPort = connection_details.substr(end + 1);
			
			std::getline(file, client_name);

			std::getline(file, filename);

			
			if (serverIP.empty() || serverPort.empty() ||
				client_name.empty() || filename.empty()) {
				file.close();
				return false; // given name is larger than allowed
			}

			file.close();
			return true;
		}
		catch (const std::exception& e) {
			// error operating in server info file
			file.close();
			std::cout << e.what() << std::endl;
			return false;
		}
	}
	catch (const std::exception& e) {
		// error opening server info file
		std::cout << e.what() << std::endl;
		return false;
	}
}

// print an exception fully in case of a nested exception
void print_exception(const std::exception& e, int level = 0)
{
	std::cout << std::string(level, ' ') << "exception: " << e.what() << std::endl;
	try {
		std::rethrow_if_nested(e);
	}
	catch (const std::exception& nestedException) {
		print_exception(nestedException, level + 1);
	}
	catch (...) {}
}

int main(int argc, char* argv[]) {

	std::string server_info = "transfer.info";
	std::string client_info = "me.info";

	std::string serverIP;
	std::string serverPort;
	std::string clientName;
	std::string filename;

	if (!readServerInfo(server_info, serverIP,
		serverPort, clientName, filename)) {
		std::cout << server_info <<
			" could not be read, client cannot be initiated" << std::endl;
		return 1;
	}

	Client client(serverIP, serverPort, clientName, client_info);
	// follow the protocol, for every serverside error - retry up to three times
	unsigned int attempt = 0;
	bool success = false;
	std::string serverside_error = "server responded with an error";

	while (!success) {
		try {
			if (!client.registerToServer()) {
				// failed to register, user is already registered
				return 1;
			}
			// client successfully interacted with the server, exit the loop
			success = true;
		}
		catch (const std::exception& e) {
			// client has encountered a serverside error
			if (attempt >= Protocol::connectionRetries) {
				// client encountered final serverside error, end the program
				print_exception(e);
				return 1;
			}
			// client still has some attempts left,
			// send general message to output and try again
			attempt++;
			std::cout << serverside_error << std::endl;
		}
	}

	// reset variables
	attempt = 0;
	success = false;
	while (!success) {
		try {
			if (!client.sendKey()) {
				// not possible in the current protocol, but added for completion
				return 1;
			}
			// client successfully interacted with the server, exit the loop
			success = true;
		}
		catch (const std::exception& e) {
			// client has encountered a serverside error
			if (attempt >= Protocol::connectionRetries) {
				// client encountered final serverside error, end the program
				print_exception(e);
				return 1;
			}
			// client still has some attempts left,
			// send general message to output and try again
			attempt++;
			std::cout << serverside_error << std::endl;
		}
	}

	// reset variables
	attempt = 0;
	unsigned int resends = 0;
	success = false;
	while (!success) {
		try {
			if (client.sendFile(filename)) {
				// client has received a response with the correct checksum
				success = true;
			}
			else {
				// client received a response with an incorrect checksum
				if (resends >= Protocol::cksumRetries) {
					// final file attempt failed - send an abort message
					client.rejectCRC(filename);
					return 1;
				}
				// attempt to resend the file
				client.retryCRC(filename);
				resends++;
			}
		}
		catch (const std::exception& e) {
			// client has encountered a serverside error
			if (attempt >= Protocol::connectionRetries) {
				// client encountered final serverside error, end the program
				print_exception(e);
				return 1;
			}
			// client still has some attempts left,
			// send general message to output and try again
			attempt++;
			std::cout << serverside_error << std::endl;
		}
	}

	// reset variables
	attempt = 0;
	success = false;
	while (!success) {
		try {
			if (!client.acceptCRC(filename)) {
				// not possible in the current protocol, but added for completion
				return 1;
			}
			// client successfully interacted with the server, exit the loop
			success = true;
		}
		catch (const std::exception& e) {
			// client has encountered a serverside error
			if (attempt >= Protocol::connectionRetries) {
				// client encountered final serverside error, end the program
				print_exception(e);
				return 1;
			}
			// client still has some attempts left,
			// send general message to output and try again
			attempt++;
			std::cout << serverside_error << std::endl;
		}
	}

	// client successfully interacted with the server fully
	return 0;
}