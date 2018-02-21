//CLIENT SENDS TO SERVER
#include <iostream>
#include <mutex>
#include <queue>
#include <fstream>
#include <ctime>
#include "asio.hpp"



using asio::ip::tcp;
class ClientUnit {
private:
	tcp::resolver resolver_;
	tcp::resolver::query query_;
	tcp::resolver::iterator endpoints_;
	tcp::socket socket_;
	asio::io_service& io_service_;

public:
	ClientUnit(asio::io_service& io_service, char * host, char * port):
	io_service_(io_service), socket_(io_service), resolver_(io_service), query_(host, port){
		endpoints_ = resolver_.resolve(query_);
		asio::connect(socket_, endpoints_);
	}

	void send(char * message_){
		asio::error_code ec;
		//build header
		std::size_t body_length_ = std::strlen(message_);
		char header_[7];
		sprintf(header_, "%7d", static_cast<int>(body_length_));
		//build body
		char body_[(int)body_length_];
		strcpy(body_, message_);
		
		//convert header+body into one string
		char send_this_[7 + (int)body_length_ + 1];
		strcpy(send_this_, header_);
		strcat(send_this_, body_);
		
		//set delimeter
		send_this_[7+(int)body_length_] = '\0';	
		
		//write to socket
		asio::write(socket_, asio::buffer((std::string) send_this_),
			asio::transfer_all(), ec);

	}
};


int main(int argc, char ** argv){
	try{
		if (argc != 3){
			std::cerr << "Usage: client <hostIP> <port>" << std::endl;
			return 1;
		}
		asio::io_service ios_;
		ClientUnit client1(ios_, argv[1], argv[2]);
		std::string input;

		std::cout << "Each new line will be sent to the server." << std::endl << "When you are finished type 'done'" << std::endl;
		
		while(getline(std::cin, input) && input != "done"){
			
		/*	 //junk code used to test algo on text files
			 clock_t time;
			 time = clock();
			 int length;
			 FILE * t = fopen("testfile", "r");
			 fseek(t, 0, SEEK_SET);
			 fseek(t, 0, SEEK_END);
			 length = ftell(t);
			 fseek(t,0,SEEK_SET);
			 char temp_[length + 1];
			 fread(temp_, length, 1, input.c_str);			*/				
			char temp_[input.length() + 1];
			std::strcpy(temp_, input.c_str());			
			temp_[input.length()] = '\0';
			std::cout << temp_ << std::endl;
			client1.send(temp_);
		}
		//	time = clock() - time;
	
			//std::cout << "post send, time in seconds took :" << ((float)time/CLOCKS_PER_SEC) << std::endl << "length of file: " << length << std::endl;
	//	}
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}

	return (0);
}
	
