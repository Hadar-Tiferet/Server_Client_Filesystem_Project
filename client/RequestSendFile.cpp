#include "RequestSendFile.h"
#include "Utility.h"
#include "ConnectionUtility.h"
#include "AESWrapper.h"
#include <fstream>
#include <iostream>
#include "CustomCRC.h"


void RequestSendFile::calculateCRC() {
	try {
		CRC crc;
		this->crcValue = crc.calcCrc(this->filepath);
	}
	catch (...) {
		// error calculating crc value for file
		std::throw_with_nested(std::invalid_argument(
			"failed to calculate CRC value for " + this->filepath));
	}
}

RequestSendFile::RequestSendFile(std::string client_id, 
	std::string aesKey, std::string filepath)
	: Request(client_id, Protocol::request_sendFile) {
	if (!Protocol::verify_file_name(filepath)) {
		// file name is too large for the protocol or contains non ascii characters
		throw std::invalid_argument("file name does not fit the protocol");
	}
	this->filepath = filepath;
	// copy the aes key for encryption
	if (!Protocol::verify_aes_key(aesKey)) {
		// aes key is not in the correct size according to the protocol
		throw std::invalid_argument("existing aes key does not fit the protocol");
	}
	this->aesKey = aesKey;

	calculateCRC();
	Utility::stringToChar(client_id, 
		this->payload.client_id, Protocol::size_client_id);
	Utility::stringToChar(this->filepath,
		this->payload.file_name, Protocol::size_file_name);

	// update the size of the payload in the header, before adding encrypted file size
	this->header.payload_size = sizeof(payload);
}

void RequestSendFile::sendEncryptedFile(boost::asio::ip::tcp::socket* sock) {
	std::string buffer;
	// send the requested file after encryption
	char key[Protocol::size_aes_key];
	Utility::stringToChar(this->aesKey,
		key, Protocol::size_aes_key);
	AESWrapper AES((unsigned char*)key, Protocol::size_aes_key);
	try {
		// open the requested file for reading, in binary mode
		std::ifstream file(this->filepath, std::ios::in | std::ios::binary);
		try {
			file.seekg(0, file.end);
			size_t length = static_cast<size_t>(file.tellg());
			file.seekg(0, file.beg);

			// verify content size
			if (length == 0 || length > UINT32_MAX) {
				// file is larger than the protocol supports, reject the request
				throw std::invalid_argument(
					"file is larger than max size supported by the protocol");
			}

			// read whole contents of the file to a buffer at once
			buffer.resize(length);
			file.read(&buffer[0], buffer.size());
			file.close();
		}
		catch (...) {
			// generic error with file open
			file.close();
			std::throw_with_nested(std::invalid_argument(
				"Failed to process file " + this->filepath));
		}
	}
	catch (const std::invalid_argument) {
		// generic error with file open
		throw;
	}
	catch (...) {
		// error
		std::throw_with_nested(std::invalid_argument(
			"Failed to open file " + this->filepath));
	}
	// encrypt the binary data
	try {
		buffer = AES.encrypt(buffer.c_str(), buffer.length());
		// make sure the size of the encrypted file fits the protocol
		if (buffer.length() > UINT32_MAX) {
			// file after encryption is larger than the protocol supports,
			// reject the request
			throw std::invalid_argument(
				"encrypted file larger than maximum supported by the protocol");
		}
	}
	catch (...) {
		// experienced an error while encrypting the file
		std::throw_with_nested(std::runtime_error(
			"Failed to encrypt file"));
	}
	// send encrypted file to server
	try {
		// update the request to reflect the size of file after encryption
		this->payload.content_size = buffer.length();
		this->header.payload_size += this->payload.content_size;
		// send the request to the server
		ConnectionUtility::sendBytes(sock,
			(char*)&this->header, sizeof(this->header));
		ConnectionUtility::sendBytes(sock,
			(char*)&this->payload, sizeof(this->payload));
		// can try sock.write_some instead and sent the string directly
		ConnectionUtility::sendBytes(sock,
			(char*)buffer.c_str(), buffer.length());
	}
	catch (...) {
		// experienced an error with connection to server
		std::throw_with_nested(std::runtime_error(
			"Failed to send file to server"));
	}
}

void RequestSendFile::send(boost::asio::ip::tcp::socket* sock) {
	sendEncryptedFile(sock);
}

const uint32_t RequestSendFile::getCRCvalue() {
	return this->crcValue;
}

const uint32_t RequestSendFile::getEncryptedFilesize() {
	return this->payload.content_size;
}
