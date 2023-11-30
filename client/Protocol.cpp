#include "Protocol.h"


bool Protocol::verify_client_id(const std::string client_id) {
	return client_id.length() == Protocol::size_client_id;
}

bool Protocol::verify_version(uint8_t version) {
	return true;
}

bool Protocol::verify_code(uint16_t code) {
	return true;
}

bool Protocol::verify_payload_size(uint32_t payload_size) {
	return true;
}

bool Protocol::verify_name(const std::string client_name) {
	if (client_name.length() > Protocol::size_name_max) {
		// name is too large for the protocol
		return false;
	}
	
	return find_if(client_name.begin(), client_name.end(),
		[](char c) { return !(isalnum(c) || (c == ' ')); }) == client_name.end();
}

bool Protocol::verify_public_key(const std::string public_key) {
	return public_key.length() == Protocol::size_public_key;
}

bool Protocol::verify_aes_key(const std::string aes_key) {
	return aes_key.length() == Protocol::size_aes_key;
}

bool Protocol::verify_content_size(uint32_t content_size) {
	return true;
}

bool Protocol::verify_file_name(const std::string file_name) {
	if (file_name.length() >= Protocol::size_file_name)
		return false;
	// find out if file_name contains any non ascii characters
	for (auto c : file_name) {
		if (static_cast<unsigned char>(c) > 127) {
			return false;
		}
	}
	return true;
}

bool Protocol::verify_cksum(uint32_t cksum) {
	return true;
}