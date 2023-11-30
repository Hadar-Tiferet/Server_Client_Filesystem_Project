#include "ResponseKeyExchange.h"
#include "Utility.h"
#include "ConnectionUtility.h"
#include "RSAWrapper.h"
#include "Base64Wrapper.h"

ResponseKeyExchange::ResponseKeyExchange(uint32_t payload_size, 
	std::string clientside_client_id, std::string rsaPrivateHex)
	: Response(payload_size) {
	if (!Protocol::verify_client_id(clientside_client_id)) {
		throw std::invalid_argument("client id does not fit the protocol");
	}
	this->clientside_client_id = clientside_client_id;
	this->rsaPrivateHex = rsaPrivateHex;
}

void ResponseKeyExchange::process(boost::asio::ip::tcp::socket* sock) {
	if (this->payload_size < Protocol::size_client_id) {
		// check for potential overflow
		throw std::invalid_argument(
			"received payload size is too small to fit the protocol");
	}

	uint32_t encryptedAESSize = this->payload_size - Protocol::size_client_id;
	
	ConnectionUtility::receiveBytes(sock, 
		this->client_id, Protocol::size_client_id);
	std::string received_client_id(this->client_id, Protocol::size_client_id);
	// verify received client id
	if (!Protocol::verify_client_id(received_client_id)) {
		throw std::invalid_argument("received client id does not fit the protocol");
	}
	if (received_client_id != this->clientside_client_id) {
		// client id received from server does not match previous info
		throw std::invalid_argument("response contains an incorrect client id");
	}
	std::string encryptedAES;
	encryptedAES.resize(encryptedAESSize);
	ConnectionUtility::receiveBytes(sock, &encryptedAES[0], encryptedAESSize);
	// decrypt the aes key received from the server using client's rsa private key
	RSAPrivateWrapper rsaDecryptor(Base64Wrapper::decode(this->rsaPrivateHex));
	this->aesKey = rsaDecryptor.decrypt(encryptedAES.c_str(), encryptedAESSize);
	// need to verify payload contents
	if (!Protocol::verify_aes_key(this->aesKey)) {
		// aes key does not fit the protocol
		throw std::invalid_argument("received aes key does not fit the protocol");
	}
}

std::string ResponseKeyExchange::getAESKey() {
	return this->aesKey;
}