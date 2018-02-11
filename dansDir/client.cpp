//CLIENT THAT READS FROM SOCKET A HEADER AND MESSAGE
#include <iostream>
#include "asio.hpp"



using asio::ip::tcp;


int main(int argc, char * argv[]){
	try{
		if (argc != 3){
			std::cerr << "Usage: client <hostIP> <port>" << std::endl;
			return 1;
		}
		
		//establish - an io_service
		asio::io_service io_service;
		
		//translate the server name into a tcp endpoint
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(argv[1], argv[2]);
		
		//return the list of endpoints
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		
		//create the socket
		tcp::socket socket_(io_service);
		
		//connect to endpoint
		asio::connect(socket_, endpoint_iterator);
		
		//create a buffer to hold the input
		asio::error_code ec;
		char header_[4];
	
		int body_length_;
		int bytes_read_;
		
		//read - fixed size - header from socket - header tells us how long the message is going to be
		bytes_read_ = socket_.read_some(asio::buffer(header_, 4), ec);
		//check error codes from read
		if (ec == asio::error::eof){
			printf("ERROR: EOF REACHED\n");
			return 1;
		}
		else if (ec) {
			printf("ERROR THROWN\n");
			throw asio::system_error(ec);
		}

		//check if we read correct length of the header
		if(bytes_read_ != 4){
			printf("Incorrect amount of bytes read from getting header\n");
			return 1;
		}
		
		//read the message
		body_length_ = atoi(header_);
		char message_[body_length_];
		bytes_read_ = socket_.read_some(asio::buffer(message_, body_length_));
		//check read amount
		if(bytes_read_ != body_length_){
			printf("Incorrect amount of bytes read from getting body\n");
			return 1;
		}
		if (ec == asio::error::eof){
			printf("ERROR: EOF REACHED\n");
			return 1;
		}
		else if (ec) {
			printf("ERROR THROWN\n");
			throw asio::system_error(ec);
		}
	std::cout << "Message Received: " << message_ << std::endl;
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

		
	return (0);
}
	
