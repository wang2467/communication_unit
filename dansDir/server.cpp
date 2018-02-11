#include <ctime>
#include <iostream>
#include <string>
#include "asio.hpp"




using asio::ip::tcp;
int main(int argc, char ** argv){
	try{
		if(argc != 3){
			printf("Usage: <port#> <message to send to client>\n");
			return 1;
		}
		asio::io_service ios_;
		//establish endpoint to listen on
		tcp::acceptor acceptor(ios_, tcp::endpoint(tcp::v4(), std::atoi(argv[1])));
		//create socket
		tcp::socket socket_(ios_);
		//establish connection
		acceptor.accept(socket_);
		//collect message to send
		std::string message = argv[2];
		//error handler
		asio::error_code ignored_error;

		//collect number of bytes of the message
		std::size_t body_length_ = std::strlen(argv[2]);
		char header_[4];
		sprintf(header_, "%4d", static_cast<int>(body_length_));
		//write the message to the socket for the client to receive
		//read the str to send into message_
		char body_[(int)body_length_];
		strcpy(body_, argv[2]);
		//combine the header with the body into one string
		char message_[4 + (int)body_length_ + 1];
		strcpy(message_, header_);
		strcat(message_, body_);
			
		//cast the message to send to type 'string' for asio::write
		asio::write(socket_, asio::buffer((std::string) message_),
			asio::transfer_all(), ignored_error);
		std::cout << "sending : " "'" << message_ << "'" << std::endl;
	}
	catch(std::exception& e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
