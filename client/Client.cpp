#include <boost/asio.hpp>
#include "Client.h"
#include <fstream>
#include <iostream>
#include "Utility.h"
#include "ConnectionUtility.h"
#include <iomanip>
#include "RequestRegistration.h"
#include "RequestSendKey.h"
#include "RequestSendFile.h"
#include "RequestAcceptCRC.h"
#include "RequestRetryCRC.h"
#include "RequestRejectCRC.h"
#include "ResponseRegistrationSuccess.h"
#include "ResponseRegistrationFail.h"
#include "ResponseKeyExchange.h"
#include "ResponseFileReceived.h"
#include "ResponseFileVerified.h"

Client::Client(std::string serverIP, std::string serverPort,
	std::string client_name, std::string client_info) {
	if (!Protocol::verify_name(client_name)) {
		throw std::invalid_argument("client name does not fit the protocol");
	}
	this->client_name = client_name;
	this->serverIP = serverIP;
	this->serverPort = serverPort;
	this->client_info = client_info;
	this->client_id = std::string(Protocol::size_client_id, '\0');
}

bool Client::read_client_info() {
	try {
		// open the client info file and attempt to read client details
		std::ifstream file(this->client_info, std::ios::in);
		try {
			if (!file.good()) {
				// file does not exist
				return false;
			}
			// variables for items read from file
			std::string client_name;
			std::getline(file, client_name);
			if (!Protocol::verify_name(client_name)) {
				file.close();
				return false; // given name is larger than allowed
			}
			if (client_name != this->client_name) {
				// client name in client info file does not match known info
				file.close();
				return false;
			}

			std::string hex_id;
			std::getline(file, hex_id);
			if (hex_id.length() != Protocol::size_client_id * 2) {
				// client id in hex does not fit the size specified in the protocol
				file.close();
				// id from file could not be converted back, wrong size
				return false;
			}
			Utility::hexstringToString(hex_id, this->client_id);
			
			file.close();
			return true;
		}
		catch (...) {
			// error operating in client info file
			file.close();
			return false;
		}
	}
	catch (...) {
		// error opening client info file
		return false;
	}
}

void Client::write_client_info(std::string rsa_private_hex) {
	try {
		// open the client info file and attempt to write client details
		std::ofstream file(this->client_info, std::ios::out);
		try {
			// write data into the client info file
			file << this->client_name << std::endl;
			
			// write the client id to file, converted to a hex string
			for (const auto &item : this->client_id) {
				file << std::setfill('0') << std::setw(2) << 
					std::hex << unsigned int((unsigned char)item);
			}
			file << std::endl;

			file << rsa_private_hex;
			
			file.close();
		}
		catch (...) {
			// error operating in client info file
			file.close();
			std::throw_with_nested(std::invalid_argument(
				"failed to write client info to " + this->client_info));
		}
	}
	catch (...) {
		// error opening client info file
		std::throw_with_nested(std::invalid_argument(
			"failed to open client info file " + this->client_info));
	}
}

bool Client::registerToServer() {
	if (read_client_info()) {
		// successfully read client information from file, no need to register again
		return true;
	}

	// client file does not exist, send a request to receive details from server
	boost::asio::io_context io_context;
	boost::asio::ip::tcp::socket sock(io_context);
	ConnectionUtility::establishConnection(&sock, &io_context,
		this->serverIP, this->serverPort);
	try {
		RequestRegistration request(this->client_id, this->client_name);
		request.send(&sock);
		// for ease of operation tracking
		std::cout << "sent a registration request to server" << std::endl;
		// attempt to receive header of a response
		server_header responseHeader;
		ConnectionUtility::receiveBytes(&sock,
			(char*)&responseHeader, sizeof(responseHeader));

		if (responseHeader.code == ResponseRegistrationSuccess::code) {
			ResponseRegistrationSuccess response(responseHeader.payload_size);
			response.process(&sock);
			// update client side client id
			this->client_id = response.getClientID();
			// update client info file to contain the received client id
			write_client_info("");

			// request to register was processed and handled correctly
			ConnectionUtility::closeConnection(&sock, &io_context);
			// for ease of operation tracking
			std::cout <<
				"received a registration success response from server" << std::endl;
			return true;
		}
		else if (responseHeader.code == ResponseRegistrationFail::code) {
			ResponseRegistrationFail response(responseHeader.payload_size);
			response.process(&sock);
			// received a valid response from the server
			ConnectionUtility::closeConnection(&sock, &io_context);
			// client is already registered, registration failed
			// for ease of operation tracking
			std::cout <<
				"received a registration failed response from server" << std::endl;
			return false;
		}
		else {
			// received a response which does not fit the current request
			throw std::invalid_argument("received an unexpected response");
		}
	}
	catch (...) {
		// encountered an error
		ConnectionUtility::closeConnection(&sock, &io_context);
		std::throw_with_nested(std::invalid_argument(
			"error encountered in registerToServer"));
	}
}

bool Client::sendKey() {
	boost::asio::io_context io_context;
	boost::asio::ip::tcp::socket sock(io_context);
	ConnectionUtility::establishConnection(&sock, &io_context, 
		this->serverIP, this->serverPort);
	try {
		RequestSendKey request(this->client_id, this->client_name);
		request.send(&sock);
		std::string rsa_private_hex = request.getRsaPrivateHex();
		write_client_info(rsa_private_hex);
		// for ease of operation tracking
		std::cout <<
			"sent a key exchange request to server" << std::endl;

		// attempt to receive header of a response
		server_header responseHeader;
		ConnectionUtility::receiveBytes(&sock, 
			(char*)&responseHeader, sizeof(responseHeader));

		if (responseHeader.code == ResponseKeyExchange::code) {
			ResponseKeyExchange response(responseHeader.payload_size, 
				this->client_id, rsa_private_hex);
			response.process(&sock);
			// update aes key
			this->aesKey = response.getAESKey();
			// request to exchange keys was processed and handled correctly
			ConnectionUtility::closeConnection(&sock, &io_context);
			// for ease of operation tracking
			std::cout <<
				"received a key exchange response from server" << std::endl;
			return true;
		}
		else {
			// received a response which does not fit the current request
			throw std::invalid_argument("received an unexpected response");
		}
	}
	catch (...) {
		// encountered an error
		ConnectionUtility::closeConnection(&sock, &io_context);
		std::throw_with_nested(std::invalid_argument("error encountered in sendKey"));
	}
}

bool Client::sendFile(std::string filepath) {
	boost::asio::io_context io_context;
	boost::asio::ip::tcp::socket sock(io_context);
	ConnectionUtility::establishConnection(&sock, &io_context, 
		this->serverIP, this->serverPort);
	try {
		RequestSendFile request(this->client_id, this->aesKey, filepath);
		request.send(&sock);
		const uint32_t crcValue = request.getCRCvalue();
		const uint32_t encryptedFilesize = request.getEncryptedFilesize();
		// for ease of operation tracking
		std::cout <<
			"sent a send file request to server" << std::endl;

		// attempt to receive header of a response
		server_header responseHeader;
		ConnectionUtility::receiveBytes(&sock, 
			(char*)&responseHeader, sizeof(responseHeader));

		if (responseHeader.code == ResponseFileReceived::code) {
			ResponseFileReceived response(responseHeader.payload_size,
				this->client_id, encryptedFilesize, filepath);
			response.process(&sock);
			// received a valid response from the server
			ConnectionUtility::closeConnection(&sock, &io_context);
			// validate crc value
			if (crcValue == response.getReceivedCksum()) {
				// cksum matches
				// for ease of operation tracking
				std::cout <<
					"received a file received response from server - checksum match" <<
					std::endl;
				return true;
			}
			// checksum does not match
			// for ease of operation tracking
			std::cout <<
				"received a file received response from server - no checksum match" <<
				std::endl;
			return false;
		}
		else {
			// received a response which does not fit the current request
			throw std::invalid_argument(
				"received an unexpected response");
		}
	}
	catch (...) {
		// encountered an error
		ConnectionUtility::closeConnection(&sock, &io_context);
		std::throw_with_nested(std::invalid_argument(
			"error encountered in sendFile"));
	}
}

bool Client::acceptCRC(std::string filepath) {
	boost::asio::io_context io_context;
	boost::asio::ip::tcp::socket sock(io_context);
	ConnectionUtility::establishConnection(&sock, &io_context, 
		this->serverIP, this->serverPort);
	try {
		RequestAcceptCRC request(this->client_id, filepath);
		request.send(&sock);
		// for ease of operation tracking
		std::cout <<
			"sent an accept crc request to server" << std::endl;

		// attempt to receive header of a response
		server_header responseHeader;
		ConnectionUtility::receiveBytes(&sock, 
			(char*)&responseHeader, sizeof(responseHeader));

		if (responseHeader.code == ResponseFileVerified::code) {
			ResponseFileVerified response(responseHeader.payload_size);
			response.process(&sock);
			// request to verify crc for the file was processed and handled correctly
			ConnectionUtility::closeConnection(&sock, &io_context);
			// for ease of operation tracking
			std::cout <<
				"received a file verified response from server" << std::endl;
			return true;
		}
		else {
			// received a response which does not fit the current request
			throw std::invalid_argument(
				"received an unexpected response");
		}
	}
	catch (...) {
		// encountered an error
		ConnectionUtility::closeConnection(&sock, &io_context);
		std::throw_with_nested(std::invalid_argument(
			"error encountered in acceptCRC"));
	}
}

bool Client::retryCRC(std::string filepath) {
	boost::asio::io_context io_context;
	boost::asio::ip::tcp::socket sock(io_context);
	ConnectionUtility::establishConnection(&sock, &io_context, this->serverIP, this->serverPort);
	try {
		RequestRetryCRC request(this->client_id, filepath);
		request.send(&sock);
		ConnectionUtility::closeConnection(&sock, &io_context);
		// for ease of operation tracking
		std::cout <<
			"sent a retry crc request to server" << std::endl;
		return true;
	}
	catch (...) {
		// encountered an error
		ConnectionUtility::closeConnection(&sock, &io_context);
		std::throw_with_nested(std::invalid_argument(
			"error encountered in retryCRC"));
	}
}

bool Client::rejectCRC(std::string filepath) {
	boost::asio::io_context io_context;
	boost::asio::ip::tcp::socket sock(io_context);
	ConnectionUtility::establishConnection(&sock, &io_context, this->serverIP, this->serverPort);
	try {
		RequestRejectCRC request(this->client_id, filepath);
		request.send(&sock);
		ConnectionUtility::closeConnection(&sock, &io_context);
		// for ease of operation tracking
		std::cout <<
			"sent a reject crc request to server" << std::endl;
		return true;
	}
	catch (...) {
		// encountered an error
		ConnectionUtility::closeConnection(&sock, &io_context);
		std::throw_with_nested(std::invalid_argument(
			"error encountered in rejectCRC"));
	}
}