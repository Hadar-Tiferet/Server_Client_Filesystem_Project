#include "ConnectionUtility.h"

void ConnectionUtility::establishConnection(boost::asio::ip::tcp::socket* sock,
	boost::asio::io_context* io_context, std::string serverIP, std::string serverPort) {
	try {
		boost::asio::ip::tcp::resolver resolver(*io_context);
		boost::asio::connect(*sock,
			resolver.resolve(serverIP, serverPort));
	}
	catch (...) {
		// error connecting to server
		std::throw_with_nested(std::runtime_error(
			"failed to establish a connection to the server"));
	}
}

void ConnectionUtility::closeConnection(boost::asio::ip::tcp::socket* sock,
	boost::asio::io_context* io_context) {
	(*sock).shutdown(boost::asio::ip::tcp::socket::shutdown_both);
	(*sock).close();
	(*io_context).stop();
}

void ConnectionUtility::sendBytes(boost::asio::ip::tcp::socket* sock,
	char* buffer, size_t size) {
	try {
		// attempt to send the specified amount of bytes through the socket
		boost::asio::write(*sock, boost::asio::buffer(
			buffer, size));
	}
	catch (...) {
		// error sending data to socket
		std::throw_with_nested(std::runtime_error(
			"Failed to send data to the server"));
	}
}

void ConnectionUtility::receiveBytes(boost::asio::ip::tcp::socket* sock,
	char* buffer, size_t size) {
	try {
		// attempt to receive exactly size bytes through the socket
		size_t received = 0;
		while (received < size) {
			size_t remaining = size - received;
			received += boost::asio::read(*sock, boost::asio::buffer(
				buffer + received, remaining));
		}
	}
	catch (...) {
		// error receiving data from socket
		std::throw_with_nested(std::runtime_error(
			"Failed to receive data from server"));
	}
}