#include "RequestRegistration.h"
#include "Utility.h"
#include "ConnectionUtility.h"

RequestRegistration::RequestRegistration(std::string client_id, std::string client_name)
	: Request(client_id, Protocol::request_registration) {
	if (!Protocol::verify_name(client_name)) {
		throw std::invalid_argument("client name does not fit the protocol");
	}
	Utility::stringToChar(client_name, 
		this->payload.name, Protocol::size_name);
	// specify the size of the payload in the header
	this->header.payload_size = sizeof(payload);
}

void RequestRegistration::send(boost::asio::ip::tcp::socket* sock) {
	ConnectionUtility::sendBytes(sock, 
		(char*)&this->header, sizeof(this->header));
	ConnectionUtility::sendBytes(sock, 
		(char*)&this->payload, sizeof(this->payload));
}