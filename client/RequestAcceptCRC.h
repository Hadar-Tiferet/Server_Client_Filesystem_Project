// RequestAcceptCRC.h

#ifndef REQUESTACCEPTCRC_H
#define REQUESTACCEPTCRC_H

#include "Request.h"

class RequestAcceptCRC : public Request {
	std::string filepath;

	// using pragma pack to specify that the memory should be used as is, unpadded
#pragma pack(push, 1)
	struct {
		char client_id[Protocol::size_client_id];
		char file_name[Protocol::size_file_name];
	}payload;
#pragma pack(pop)

public:
	RequestAcceptCRC(std::string client_id, std::string filepath);
	virtual void send(boost::asio::ip::tcp::socket* sock);
};

#endif // RequestAcceptCRC.h