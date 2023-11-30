#ifndef CONNECTIONUTILITY_H
#define CONNECTIONUTILITY_H

#include <boost/asio.hpp>

class ConnectionUtility {
public:
	static void establishConnection(boost::asio::ip::tcp::socket* sock,
		boost::asio::io_context* io_context, std::string serverIP, std::string serverPort);
	static void closeConnection(boost::asio::ip::tcp::socket* sock,
		boost::asio::io_context* io_context);
	static void sendBytes(boost::asio::ip::tcp::socket* sock,
		char* buffer, size_t size);
	static void receiveBytes(boost::asio::ip::tcp::socket* sock,
		char* buffer, size_t size);
};

#endif