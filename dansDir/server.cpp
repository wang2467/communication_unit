#include <ctime>
#include <iostream>
#include <string>
#include <mutex>
#include <queue>
#include <thread>
#include <ctime>
#include "asio.hpp"

using asio::ip::tcp;


class ServerUnit{

public:
	//constructor
	ServerUnit(asio::io_service& io_service_, short port_)
	:acceptor_(io_service_, tcp::endpoint(tcp::v4(), port_)), socket_(io_service_){
		for(;;) { start_accept(port_); }//keep open forever
	}

private:
	
	//accept connection with client trying to merge on port
	void start_accept(short port_){
		std::cout << "Making connection on port " << port_ << std::endl;
		acceptor_.accept(socket_);
		int check;
		for(;;){ 
			if(start_reading() == EXIT_FAILURE){//connection closed by peer - must find new connection
				break;
			}
		}
		
		std::cout << "Stopped Reading... Disconnecting from port "<< port_ << std::endl;
		//close socket so we can search for new peer
		socket_.close();
	}


	//begin a continuous loop of reading from the socket
	int start_reading(){
		//read header
		std::cout << "Reading..." << std::endl;
		char header_[8] = {'0','0','0','0','0','0','0','\0'};
		int bytes_read_ = socket_.read_some(asio::buffer(header_, 7), ec);
		//set delimeter
		header_[7]='\0';
		//wrong amount of bytes read
		if(bytes_read_ != 7 && bytes_read_ != 0){
			std::cout << "Incorrect number of bytes read when reading header" << std::endl;
		}
		
		if(ec == asio::error::eof){//connection closed by peer
			std::cout << "ERROR: EOF REACHED" << std::endl;
			return EXIT_FAILURE;
		}
		//read body
		int body_length_ = atoi(header_);
		char body_[body_length_ + 1] = {'0'};
		bytes_read_ = socket_.read_some(asio::buffer(body_, body_length_));

		//read_some may not have read the full length 
		//designated by the header because of OS? 
		//limitations.. We need to make sure we 
		//read the entire body ------ That is 
		//what this while loop does
		while (bytes_read_ != body_length_ && ec != asio::error::eof){
			bytes_read_ += socket_.read_some(asio::buffer(body_,body_length_ - bytes_read_));
		}
		//set delimeter
		body_[body_length_] = '\0';
		//wrong amount of bytes read
		if(bytes_read_ != body_length_){
			std::cout << "Incorrect number of bytes read when reading body: " << bytes_read_ << "body length" << body_length_ << std::endl;
			return EXIT_FAILURE;
		}
		if(ec == asio::error::eof){//connection closed by peer
			std::cout << "ERROR: EOF REACHED" << std::endl;
			return EXIT_FAILURE;
		}
	std::cout << "Message Received: " << body_ << std::endl;

	return EXIT_SUCCESS;
	}


	//variables
	asio::error_code ec;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	char * totalString;
};
void buildThreadForReading(short port){
	asio::io_service ios_;
	ServerUnit s(ios_, port);
	ios_.run();
}
int main(int argc, char ** argv){
	try{
		if(argc != 2){
			std::cout << "Usage: <port#>" << std::endl;
			return 1;
		}
		std::thread f1 (buildThreadForReading, atoi(argv[1]));
//		std::thread f2 (buildThreadForReading, <arbitrary port number>);
		f1.join();
//		f2.join();
	}
	catch(std::exception& e){
		std::cerr << e.what() << std::endl;
	}
	return 0;
}
