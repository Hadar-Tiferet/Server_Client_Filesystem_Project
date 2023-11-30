// ResponseRegistrationSuccess.h

#ifndef RESPONSEREGISTRATIONSUCCESS_H
#define RESPONSEREGISTRATIONSUCCESS_H

#include "Response.h"

class ResponseRegistrationSuccess : public Response {
	std::string received_client_id;
	// using pragma pack to specify that the memory should be used as is, unpadded
#pragma pack(push, 1)
	struct {
		char client_id[Protocol::size_client_id];
	}payload;
#pragma pack(pop)

public:
	ResponseRegistrationSuccess(uint32_t payload_size);
	virtual void process(boost::asio::ip::tcp::socket* sock);

	std::string getClientID();

	static const uint16_t code = Protocol::response_registrationSuccess;
};

#endif // ResponseRegistrationSuccess.h