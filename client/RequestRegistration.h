// RequestRegistration.h

#ifndef REQUESTREGISTRATION_H
#define REQUESTREGISTRATION_H

#include "Request.h"

class RequestRegistration : public Request {
	// using pragma pack to specify that the memory should be used as is, unpadded
	#pragma pack(push, 1)
	struct {
		char name[Protocol::size_name];
	}payload;
	#pragma pack(pop)

public:
	RequestRegistration(std::string client_id, std::string client_name);
	virtual void send(boost::asio::ip::tcp::socket* sock);
};

#endif // RequestRegistration.h