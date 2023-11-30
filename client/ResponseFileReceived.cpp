#include "ResponseFileReceived.h"
#include "Utility.h"
#include "ConnectionUtility.h"
#include <iostream>

ResponseFileReceived::ResponseFileReceived(uint32_t payload_size, 
	std::string clientside_client_id, uint32_t clientside_content_size,
	std::string clientside_filepath)
	: Response(payload_size) {
	if (!Protocol::verify_client_id(clientside_client_id)) {
		throw std::invalid_argument("client id does not fit the protocol");
	}
	this->clientside_client_id = clientside_client_id;
	this->clientside_content_size = clientside_content_size;
	if (!Protocol::verify_file_name(clientside_filepath)) {
		throw std::invalid_argument("file path does not fit the protocol");
	}
	this->clientside_file_name = clientside_filepath;
}

void ResponseFileReceived::process(boost::asio::ip::tcp::socket* sock) {
	if (this->payload_size != sizeof(this->payload))
		throw std::invalid_argument("received payload size does not fit the protocol");
	ConnectionUtility::receiveBytes(sock, 
		(char*)&this->payload, this->payload_size);
	// verify payload contents
	std::string received_client_id(this->payload.client_id, Protocol::size_client_id);
	// verify client id
	if (!Protocol::verify_client_id(received_client_id)) {
		throw std::invalid_argument("received client id does not fit the protocol");
	}
	if (received_client_id != this->clientside_client_id) {
		// client id received from server does not match previous info
		throw std::invalid_argument("response contains an incorrect client id");
	}
	
	// verify content size
	if (this->clientside_content_size != this->payload.content_size) {
		// response contains a different encrypted file size - response is invalid
		throw std::invalid_argument("response contains an incorrect file size");
	}
	
	std::string received_file_name = Utility::charToTrimmedString(
		this->payload.file_name, Protocol::size_file_name);
	// verify file name
	if (!Protocol::verify_file_name(received_file_name)) {
		throw std::invalid_argument("received file name does not fit the protocol");
	}
	if (received_file_name != this->clientside_file_name) {
		// response file name does not match the sent file
		throw std::invalid_argument("response contains an incorrect file name");
	}
}

uint32_t ResponseFileReceived::getReceivedCksum() {
	return this->payload.cksum;
}