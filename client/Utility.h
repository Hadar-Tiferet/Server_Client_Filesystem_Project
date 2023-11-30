#ifndef UTILITY_H
#define UTILITY_H

#include <string>

class Utility {
public:
	static void clearBuffer(char* buffer, size_t size);
	static void stringToChar(std::string string, char* buffer, size_t bufferSize);
	static bool hexstringToString(const std::string& hexstring, std::string& result);
	static std::string charToTrimmedString(char* buffer, size_t bufferSize);
};

#endif