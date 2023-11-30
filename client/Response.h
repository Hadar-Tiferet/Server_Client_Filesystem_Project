// Response.h

#ifndef RESPONSE_H
#define RESPONSE_H

#include "Protocol.h"
#include <boost/asio.hpp>

class Response {
protected:
	uint32_t payload_size;
public:
	Response(uint32_t payload_size);
	virtual void process(boost::asio::ip::tcp::socket* sock) = 0; // set as abstract to contain specific implementations
	virtual ~Response();
};

#endif // Response.h