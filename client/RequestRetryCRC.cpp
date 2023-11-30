#include "RequestRetryCRC.h"
#include "Utility.h"
#include "ConnectionUtility.h"

RequestRetryCRC::RequestRetryCRC(std::string client_id, std::string filepath)
	: Request(client_id, Protocol::request_retryCRC) {
	if (!Protocol::verify_file_name(filepath)) {
		// file name is too large for the protocol
		throw std::invalid_argument("file name does not fit the protocol");
	}
	this->filepath = filepath;
	Utility::stringToChar(client_id, 
		this->payload.client_id, Protocol::size_client_id);
	Utility::stringToChar(this->filepath, 
		this->payload.file_name, Protocol::size_file_name);

	// specify the size of the payload in the header
	this->header.payload_size = sizeof(payload);
}

void RequestRetryCRC::send(boost::asio::ip::tcp::socket* sock) {
	ConnectionUtility::sendBytes(sock, 
		(char*)&this->header, sizeof(this->header));
	ConnectionUtility::sendBytes(sock, 
		(char*)&this->payload, sizeof(this->payload));
}
