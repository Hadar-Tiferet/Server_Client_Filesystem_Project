// RequestSendFile.h

#ifndef REQUESTSENDFILE_H
#define REQUESTSENDFILE_H

#include "Request.h"

class RequestSendFile : public Request {
	std::string filepath;
	std::string aesKey;
	uint32_t crcValue;

	// using pragma pack to specify that the memory should be used as is, unpadded
#pragma pack(push, 1)
	struct {
		char client_id[Protocol::size_client_id];
		uint32_t content_size;
		char file_name[Protocol::size_file_name];
	}payload;
#pragma pack(pop)

	void calculateCRC();
	void sendEncryptedFile(boost::asio::ip::tcp::socket* sock);

public:
	RequestSendFile(std::string client_id, std::string aesKey, std::string filepath);
	virtual void send(boost::asio::ip::tcp::socket* sock);
	const uint32_t getCRCvalue();
	const uint32_t getEncryptedFilesize();
};

#endif // RequestSendFile.h