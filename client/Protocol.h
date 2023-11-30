#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>

class Protocol {
public:
	static const uint8_t version = 3;
	static const unsigned int connectionRetries = 3;
	static const unsigned int cksumRetries = 3;
	// specify sizes of constant fields in the protocol
	static const unsigned int size_client_id = 16;
	static const unsigned int size_version = 1;
	static const unsigned int size_code = 2;
	static const unsigned int size_payload_size = 4;
	
	static const unsigned int size_name = 255;
	static const unsigned int size_name_max = 100;
	static const unsigned int size_public_key = 160;
	static const unsigned int size_content_size = 4;
	static const unsigned int size_file_name = 255;
	static const unsigned int size_cksum = 4;

	static const unsigned int size_aes_key = 16;
	static const unsigned int size_rsa_bits = 1024;

	static const uint16_t request_registration = 1100;
	static const uint16_t request_sendKey = 1101;
	static const uint16_t request_sendFile = 1103;
	static const uint16_t request_acceptCRC = 1104;
	static const uint16_t request_retryCRC = 1105;
	static const uint16_t request_rejectCRC = 1106;
	static const uint16_t response_registrationSuccess = 2100;
	static const uint16_t response_registrationFail = 2101;
	static const uint16_t response_keyExchange = 2102;
	static const uint16_t response_fileReceived = 2103;
	static const uint16_t response_fileVerified = 2104;


	static bool verify_client_id(const std::string client_id);
	static bool verify_version(uint8_t version);
	static bool verify_code(uint16_t code);
	static bool verify_payload_size(uint32_t payload_size);
	static bool verify_name(const std::string client_name);
	static bool verify_public_key(const std::string public_key);
	static bool verify_aes_key(const std::string aes_key);
	static bool verify_content_size(uint32_t content_size);
	static bool verify_file_name(const std::string file_name);
	static bool verify_cksum(uint32_t cksum);
};

#endif