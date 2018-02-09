//CLIENT

#include <iostream>
#include "asio.hpp"



using asio::ip::tcp;


int main(int argc, char * argv[]){
	try{
		if (argc != 3){
			std::cerr << "Usage: client <hostIP> <port>" << std::endl;
			return 1;
		}
		
		//establish an io_service
		asio::io_service io_service;
		
		//translate the server name into a tcp endpoint
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(argv[1], argv[2]);
		
		//return the list of endpoints
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		
		for(;;){
			//create and connect the socket
			tcp::socket socket_(io_service);
			asio::connect(socket_, endpoint_iterator);
			
			//create a buffer to hold the input
			asio::error_code ec;
			char buf[4];
			size_t len = socket_.read_some(asio::buffer(buf, 4), ec);	
			if (ec == asio::error::eof){
				printf("ERROR: EOF REACHED\n");
				break;
			}
			else if (ec) {
				printf("ERROR THROWN\n");
				throw asio::system_error(ec);
			}
			//std::cout.write((const char *)buf.data_, len);
		printf("Message Received: %s\n",(char *) buf);
		}
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

		
	return (0);
}
	
