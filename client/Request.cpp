#include "Request.h"
#include "Utility.h"

Request::Request(std::string client_id, uint16_t code) {
	// create the header for a request
	if (!Protocol::verify_client_id(client_id))
		throw std::invalid_argument("client name does not fit the protocol");
	Utility::stringToChar(client_id, 
		this->header.client_id, Protocol::size_client_id);
	this->header.version = Protocol::version;
	this->header.code = code;
	this->header.payload_size = 0;
}

Request::~Request() {}