#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "Protocol.h"

class Client {
	std::string client_id;
	std::string client_name;
	std::string aesKey;
	std::string client_info;
	std::string serverIP;
	std::string serverPort;
	
	// using pragma pack to specify that the memory should be used as is, unpadded
#pragma pack(push, 1)
	struct server_header {
		uint8_t version;
		uint16_t code;
		uint32_t payload_size;
	};
#pragma pack(pop)

	bool read_client_info();
	void write_client_info(std::string rsa_private_hex);

public:
	Client(std::string server_IP, std::string serverPort,
		std::string client_name, std::string client_info);
	bool registerToServer();
	bool sendKey();
	bool sendFile(std::string filepath);

	bool acceptCRC(std::string filepath);
	bool retryCRC(std::string filepathh);
	bool rejectCRC(std::string filepath);
};

#endif