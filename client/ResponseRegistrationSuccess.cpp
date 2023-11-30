#include "ResponseRegistrationSuccess.h"
#include "Utility.h"
#include "ConnectionUtility.h"

ResponseRegistrationSuccess::ResponseRegistrationSuccess(uint32_t payload_size)
	: Response(payload_size) {

}

void ResponseRegistrationSuccess::process(boost::asio::ip::tcp::socket* sock) {
	if (this->payload_size != sizeof(this->payload))
		throw std::invalid_argument("received payload size does not fit the protocol");
	ConnectionUtility::receiveBytes(sock, 
		(char*)&this->payload, this->payload_size);

	// verify payload contents
	this->received_client_id = std::string(
		this->payload.client_id, Protocol::size_client_id);
	if (!Protocol::verify_client_id(received_client_id)) {
		throw std::invalid_argument("received client id does not fit the protocol");
	}
}

std::string ResponseRegistrationSuccess::getClientID() {
	return this->received_client_id;
}