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
		tcp::acceptor acceptor(ios_, tcp::endpoint(tcp::v4(), std::atoi(argv[1])));
		for(;;){
			tcp::socket socket_(ios_);
			acceptor.accept(socket_);
			std::string message = argv[2];
			asio::error_code ignored_error;
			asio::write(socket_, asio::buffer(message),
				asio::transfer_all(), ignored_error);
		}

	}
	catch(std::exception& e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
