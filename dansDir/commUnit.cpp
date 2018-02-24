#include <ctime>
#include <iostream>
#include <string>
#include <mutex>
#include <queue>
#include <thread>
#include <ctime>
#include "asio.hpp"

using asio::ip::tcp;

class ClientUnit{
private: 
	tcp::resolver resolver_;
	tcp::resolver::query query_;
	tcp::resolver::iterator endpoints_;
	tcp::socket socket_;
	asio::io_service& io_service_;
public:
	ClientUnit(asio::io_service& io_service, char *host, char *port):
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
		
		//concatenate body with header
		char send_this_[7+(int)body_length_+1];
		strcpy(send_this_, header_);
		strcat(send_this_, body_);
		
		//set delimeter
		send_this_[7+(int)body_length_] = '\0';

		//send to socket
		asio::write(socket_, asio::buffer((std::string) send_this_),
			asio::transfer_all(), ec);
	}
};

class ServerUnit{
public:
	//constructor
	ServerUnit(asio::io_service& io_service_, short port_)
	:acceptor_(io_service_, tcp::endpoint(tcp::v4(), port_)), socket_(io_service_){
		//if the client hangs up, the server cannot
		//reconnect ~ if we want the server to always
		//look for a connection put this function
		//call in a for loop
		start_accept(port_); 
	}

private:
	//retrieve the header from socket
	void getHeader(char * header_){
		//read header of length 7 bytes into 'header_' from socket
		int bytes_read_ = socket_.read_some(asio::buffer(header_, 7), ec);
		
		//set delimeter on header_
		header_[7] = '\0';
		
		//if wrong amount of bytes read display error msg
		if(bytes_read_ != 7 && bytes_read_ != 0){
			std::cout << "Incorrect number of bytes read when reading header" << std::endl;
		}
	}

	//build body from socket
	void getBody(char *header_, char *body_){
		//get body length from the header
		header_[7] = '\0';

		int body_length_ = atoi(header_);
		int bytes_read_ = asio::read(socket_, asio::buffer(body_, body_length_));
		body_[body_length_] = '\0';
	
		if(bytes_read_ != body_length_){
			std::cout << "ERROR: incorrect number of bytes read while reading body" << std::endl;
		}
	}

	//accept connection with client trying to merge on port
	void start_accept(short port_){
		std::cout << "Making connection on port " << port_ << std::endl;
		acceptor_.accept(socket_);
		for(;;){
			//connection closed by client 
			if(start_reading() == EXIT_FAILURE){
				break;
			}
		}
		
		std::cout << "Stopped Reading... Disconnecting from port "<< port_ << std::endl;
		//close socket so we can search for new peer
		socket_.close();
	}

	//begin a continuous loop of reading from the socket
	//while the client stays connected
	int start_reading(){

		char header_[8]; 
		getHeader(header_);
		//connection closed
		if(ec == asio::error::eof){//connection closed by peer
			std::cout << "ERROR: EOF REACHED: from header" << std::endl;
			return EXIT_FAILURE;
		}
		
		char body_[atoi(header_) + 1];
		getBody(header_, body_);
		//connection closed
		if(ec == asio::error::eof){
			std::cout << "ERROR: EOF REACHED: for body" << std::endl;
			return EXIT_FAILURE;
		}


		//Write Message to file
	//	FILE *out = fopen("book_out.txt", "w");
	//	fwrite(body_, strlen(body_), 1, out);
	//	fclose(out);
		std::cout << "Message Recieved: " << body_ << std::endl;
		
		return EXIT_SUCCESS;
	}


	//variables
	asio::error_code ec;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
};
void buildThreadForServer(){
	int port_;
	std::cout << "Enter a port to connect on: ";
	scanf("%d", &port_);
	asio::io_service ios_;
	ServerUnit s(ios_, port_);
	ios_.run();
}
void buildThreadForClient(){
	char port_[20];
	char IP_[50];
	int templen;
	asio::io_service ios_;
	std::cout << "Enter a port to connect on: ";
	fgets(port_, 20, stdin);
	templen = strlen(port_);
	port_[templen-1]='\0';
	std::cout << "Enter server IP: ";
	fgets(IP_, 50, stdin);
	templen=strlen(IP_);
	IP_[templen-1]='\0';
	ClientUnit c(ios_, IP_, port_);

	//enter loop to send data
	
	std::string input;
	while(getline(std::cin, input) && input != "done"){
		char temp_[input.length() + 1];
		std::strcpy(temp_, input.c_str());
		temp_[input.length()] = '\0';
		c.send(temp_);	
	}
}
int main(int argc, char ** argv){
	try{
		if(argc != 2){
			std::cout << "Available Flags: s, c, sc " << std::endl;
			std::cout << "s for server, c for client, sc for server & client" << std::endl;

			std::cout << "Usage: <flag>" << std::endl;
			return 1;
		}
		if(strlen(argv[1]) == 1 && argv[1][0] == 's'){
			std::thread t1 = std::thread (buildThreadForServer);
			t1.join();
		}

		else if(strlen(argv[1]) == 1  && argv[1][0] == 'c'){
			std::thread t2 = std::thread (buildThreadForClient);
			t2.join();
		}

		else if(strlen(argv[1]) == 2 && argv[1][0] == 's' && argv[1][1] == 'c'){

		}

		else{
			std::cout << "ERROR: Invalid arguments" << std::endl;
			std::cout << "Available Flags: s, c, sc " << std::endl;
			std::cout << "s for server, c for client, sc for server & client" << std::endl;
		}

	}
	catch(std::exception& e){
		std::cerr << e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}
