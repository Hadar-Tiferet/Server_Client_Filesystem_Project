// ResponseFileReceived.h

#ifndef RESPONSEFILERECEIVED_H
#define RESPONSEFILERECEIVED_H

#include "Response.h"

class ResponseFileReceived : public Response {
	std::string clientside_client_id;
	uint32_t clientside_content_size;
	std::string clientside_file_name;
	// using pragma pack to specify that the memory should be used as is, unpadded
#pragma pack(push, 1)
	struct {
		char client_id[Protocol::size_client_id];
		uint32_t content_size;
		char file_name[Protocol::size_file_name];
		uint32_t cksum;
	}payload;
#pragma pack(pop)

public:
	ResponseFileReceived(uint32_t payload_size,
		std::string clientside_client_id, uint32_t clientside_content_size,
		std::string clientside_filepath);
	virtual void process(boost::asio::ip::tcp::socket* sock);

	uint32_t getReceivedCksum();

	static const uint16_t code = Protocol::response_fileReceived;
};

#endif // ResponseFileReceived.h