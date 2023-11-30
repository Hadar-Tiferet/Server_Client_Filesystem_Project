#pragma once

#include "Protocol.h"


class AESWrapper
{
public:
	static const unsigned int DEFAULT_KEYLENGTH = Protocol::size_aes_key;
private:
	unsigned char _key[DEFAULT_KEYLENGTH];
	AESWrapper(const AESWrapper& aes);
public:
	static unsigned char* GenerateKey(unsigned char* buffer, unsigned int length);

	AESWrapper();
	AESWrapper(const unsigned char* key, unsigned int size);
	~AESWrapper();

	const unsigned char* getKey() const;

	std::string encrypt(const char* plain, uint32_t length);
	std::string decrypt(const char* cipher, unsigned int length);
};