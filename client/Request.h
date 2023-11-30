// Request.h

#ifndef REQUEST_H
#define REQUEST_H

#include "Protocol.h"
#include <boost/asio.hpp>

class Request {
protected:
	// using pragma pack to specify that the memory should be used as is, unpadded
	#pragma pack(push, 1)
	struct client_header {
		char client_id[Protocol::size_client_id];
		uint8_t version;
		uint16_t code;
		uint32_t payload_size;
	}header;
	#pragma pack(pop)
public:
	Request(std::string client_id, uint16_t code);
	// set as abstract to contain specific implementations
	virtual void send(boost::asio::ip::tcp::socket* sock) = 0;
	virtual ~Request();
};

#endif // Request.h