// ResponseRegistrationFail.h

#ifndef RESPONSEREGISTRATIONFAIL_H
#define RESPONSEREGISTRATIONFAIL_H

#include "Response.h"

class ResponseRegistrationFail : public Response {

public:
	ResponseRegistrationFail(uint32_t payload_size);
	virtual void process(boost::asio::ip::tcp::socket* sock);

	static const uint16_t code = Protocol::response_registrationFail;
};

#endif // ResponseRegistrationFail.h