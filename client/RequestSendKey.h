// RequestSendKey.h

#ifndef REQUESTSENDKEY_H
#define REQUESTSENDKEY_H

#include "Request.h"

class RequestSendKey : public Request {
	std::string rsa_private_hex;
	// using pragma pack to specify that the memory should be used as is, unpadded
#pragma pack(push, 1)
	struct {
		char name[Protocol::size_name];
		char public_key[Protocol::size_public_key];
	}payload;
#pragma pack(pop)

public:
	RequestSendKey(std::string client_id, std::string name);
	virtual void send(boost::asio::ip::tcp::socket* sock);
	const std::string getRsaPrivateHex();
};

#endif // RequestSendKey.h