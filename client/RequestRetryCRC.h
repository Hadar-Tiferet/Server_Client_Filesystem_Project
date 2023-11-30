// RequestRetryCRC.h

#ifndef REQUESTRETRYCRC_H
#define REQUESTRETRYCRC_H

#include "Request.h"

class RequestRetryCRC : public Request {
	std::string filepath;

	// using pragma pack to specify that the memory should be used as is, unpadded
#pragma pack(push, 1)
	struct {
		char client_id[Protocol::size_client_id];
		char file_name[Protocol::size_file_name];
	}payload;
#pragma pack(pop)

public:
	RequestRetryCRC(std::string client_id, std::string filepath);
	virtual void send(boost::asio::ip::tcp::socket* sock);
};

#endif // RequestRetryCRC.h