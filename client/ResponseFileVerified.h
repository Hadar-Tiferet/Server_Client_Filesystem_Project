// ResponseFileVerified.h

#ifndef RESPONSEFILEVERIFIED_H
#define RESPONSEFILEVERIFIED_H

#include "Response.h"

class ResponseFileVerified : public Response {

public:
	ResponseFileVerified(uint32_t payload_size);
	virtual void process(boost::asio::ip::tcp::socket* sock);

	static const uint16_t code = Protocol::response_fileVerified;
};

#endif // ResponseFileVerified.h