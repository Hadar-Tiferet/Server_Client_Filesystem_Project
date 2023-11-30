#include "ResponseFileVerified.h"
#include "Utility.h"

ResponseFileVerified::ResponseFileVerified(uint32_t payload_size)
	: Response(payload_size) {

}

void ResponseFileVerified::process(boost::asio::ip::tcp::socket* sock) {
	if (this->payload_size != 0)
		throw std::invalid_argument("received payload size does not fit the protocol");
}
