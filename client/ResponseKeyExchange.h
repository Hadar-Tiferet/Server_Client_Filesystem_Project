// ResponseKeyExchange.h

#ifndef RESPONSEKEYEXCHANGE_H
#define RESPONSEKEYEXCHANGE_H

#include "Response.h"

class ResponseKeyExchange : public Response {
	char client_id[Protocol::size_client_id];
	std::string aesKey;

	std::string clientside_client_id;
	std::string rsaPrivateHex;

public:
	ResponseKeyExchange(uint32_t payload_size, 
		std::string clientside_client_id, std::string rsaPrivateHex);
	virtual void process(boost::asio::ip::tcp::socket* sock);

	std::string getAESKey();

	static const uint16_t code = Protocol::response_keyExchange;
};

#endif // ResponseKeyExchange.h