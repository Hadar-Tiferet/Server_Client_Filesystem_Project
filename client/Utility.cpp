#include "Utility.h"
#include <iostream>

// clear a given char arraty of its contents, filling it with null characters instead
void Utility::clearBuffer(char* buffer, size_t size) {
	// can trigger buffer overflow if not careful (size larger than buffer holds)
	for (size_t i = 0; i < size; i++)
		buffer[i] = '\0';
}

// insert the contents of a given string into a given char array, filling
// the remaining space with null characters
void Utility::stringToChar(std::string string, char* buffer, size_t bufferSize) {
	// clear the buffer from previous contents
	Utility::clearBuffer(buffer, bufferSize);  
	// copy from string to buffer, no more than the buffer size
	size_t size = string.length();
	if (bufferSize < size)
		size = bufferSize;
	for (size_t i = 0; i < size; i++)
		buffer[i] = string[i];
}

// convert a string containing hex characters into a regular string
bool Utility::hexstringToString(const std::string& hexstring, std::string& result) {
	result.clear();
	for (size_t i = 0; i < hexstring.length(); i += 2) {
		std::string byte = hexstring.substr(i, 2);
		char c = (char)(int)strtol(byte.c_str(), NULL, 16);
		result.push_back(c);
	}
	return true;
}

// convert the contents of a given char array into a string object, removing
// trailing null characters
std::string Utility::charToTrimmedString(char* buffer, size_t bufferSize) {
	std::string trimmed(buffer, bufferSize);
	trimmed.erase(std::find(
		trimmed.begin(), trimmed.end(), '\0'), trimmed.end());
	return trimmed;
}
