#include "RequestSendKey.h"
#include "Utility.h"
#include "ConnectionUtility.h"
#include "RSAWrapper.h"
#include "Base64Wrapper.h"

RequestSendKey::RequestSendKey(std::string client_id, std::string name)
	: Request(client_id, Protocol::request_sendKey) {
	if (!Protocol::verify_name(name)) {
		throw std::invalid_argument("client name does not fit the protocol");
	}
	Utility::stringToChar(name, 
		this->payload.name, Protocol::size_name);

	// generate an RSA decryptor
	RSAPrivateWrapper rsaDecryptor;
	// encode the private key in base64 and store as a string (to be decoded later)
	this->rsa_private_hex = Base64Wrapper::encode(rsaDecryptor.getPrivateKey());
	
	Utility::clearBuffer(this->payload.public_key, 
		sizeof(this->payload.public_key));
	rsaDecryptor.getPublicKey(this->payload.public_key, 
		RSAPublicWrapper::KEYSIZE);
	// specify the size of the payload in the header
	this->header.payload_size = sizeof(payload);
}

void RequestSendKey::send(boost::asio::ip::tcp::socket* sock) {
	ConnectionUtility::sendBytes(sock, 
		(char*)&this->header, sizeof(this->header));
	ConnectionUtility::sendBytes(sock, 
		(char*)&this->payload, sizeof(this->payload));
}

const std::string RequestSendKey::getRsaPrivateHex() {
	return this->rsa_private_hex;
}