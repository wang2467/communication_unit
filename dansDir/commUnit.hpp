#include <ctime>
#include <iostream>
#include <string>
#include <mutex>
#include <queue>
#include <thread>
#include <ctime>
#include "asio.hpp"
using asio::ip::tcp;
std::mutex m;

/*********************************************
* Builds a header that describes the length of
* the message we are sending to the server
*********************************************/
static void buildHeader(char *message_, char *header_){
	std::size_t body_length_ = std::strlen(message_);
	sprintf(header_, "%7d", static_cast<int>(body_length_));
}

/*************************************************
* Build a message ready to be sent to the socket
* which has a fixed sized header and a dynamically
* sized body ~ max size header can represent it 
* up to 9,999,999 bytes
*************************************************/
static void buildPacketToSend(char *message_ , char *send_this_, char *header_){
	
	//build body	
	char body_[strlen(message_)];
	strcpy(body_, message_);
	
	//concatenate body with header
	strcpy(send_this_, header_);
	strcat(send_this_, body_);
	
	//set delimeter
	send_this_[7+strlen(message_)] = '\0';
}

/*************************************************
* Sends the package with header+body to the socket
* for the server to read and decrypt
*************************************************/
void send(std::queue<char *> &outQueue, tcp::socket& socket_, asio::error_code ec){
	char * message_ = outQueue.front();
	char header_[7];
	buildHeader(message_, header_);
	
	char send_this_[8 + (int)strlen(message_)];
	buildPacketToSend(message_, send_this_, header_);
	//send to socket
	asio::write(socket_, asio::buffer((std::string) send_this_),
	asio::transfer_all(), ec);
}

/*********************************************
* Reads a fixed sized header from the socket
* so we can determine the size of he attached
* message
*********************************************/
static void getHeader(char * header_, tcp::socket& socket_, asio::error_code& ec){
	//read header of length 7 bytes from socket
	int bytes_read_ = socket_.read_some(asio::buffer(header_, 7), ec);
	
	//set delimeter on header_
	header_[7] = '\0';
	
	//check if we read wrong amount of bytes from socket
	if(bytes_read_ != 7 && bytes_read_ != 0){
		std::cout << "Incorrect number of bytes read when reading header" << std::endl;
	}
}

/******************************************
* Reads the body (message) from the socket
* whose size is representated by our header
******************************************/
static void getBody(char *header_, char *body_, tcp::socket& socket_){
	
	//get body length from the header
	header_[7] = '\0';
	
	//convert header to an integer representing incoming message length
	int body_length_ = atoi(header_);
	
	//read the message from socket
	int bytes_read_ = asio::read(socket_, asio::buffer(body_, body_length_));
	
	//add delimeter
	body_[body_length_] = '\0';
	
	//check if we read wrong about of bytes from the socket
	if(bytes_read_ != body_length_){
		std::cout << "ERROR: incorrect number of bytes read while reading body" << std::endl;
	}
}


/******************************************
* 'start_reading(--)' attemps to read from 
* the socket, if the client hangs up the
* connection, error_code reads end of file
* and we return EXIT_FAILURE
* this function is called in a continuous
* loop until the client hangs up
******************************************/
static int start_reading(tcp::socket& socket_, asio::error_code ec, std::queue<char *> &inQueue){
	char *header_ = (char *)malloc(sizeof(char)*(8));
	getHeader(header_, socket_, ec);
	//connection closed
	if(ec == asio::error::eof){//connection closed by peer
		std::cout << "ERROR: EOF REACHED: from header" << std::endl;
		return EXIT_FAILURE;
	}
	
	char *body_ = (char*)malloc(sizeof(char)*(atoi(header_) + 1));
	getBody(header_, body_, socket_);
	//connection closed
	if(ec == asio::error::eof){
		std::cout << "ERROR: EOF REACHED: for body" << std::endl;
		return EXIT_FAILURE;
	}

	//Write Message to file
	
//	FILE *out = fopen("out.txt", "w");
//	std::cout << "Message Recv:" << body_ << std::endl;
//	fwrite(body_, strlen(body_), 1, out);
//	fclose(out);
	
	//push recieved message to queue
	m.lock();
	std::cout << "Message Recieved: " << body_ << std::endl;
	inQueue.push(body_);
	m.unlock();
	return EXIT_SUCCESS;
}

/*********************************************************
* The ClientUnit is a class which initiates a client side
* connection. Every ClientUnit depends on a ServerUnit to
* connect to. ClientUnit WRITES to the socket.
*********************************************************/
class ClientUnit{
private: 
	tcp::resolver resolver_;
	tcp::resolver::query query_;
	tcp::resolver::iterator endpoints_;
	tcp::socket socket_;
	asio::io_service& io_service_;
	asio::error_code ec;
public:
	ClientUnit(asio::io_service& io_service, char *host, char *port, std::queue<char *> &inQueue, std::queue<char *> &outQueue):
	io_service_(io_service), socket_(io_service), resolver_(io_service), query_(host, port){
		endpoints_ = resolver_.resolve(query_);
		asio::connect(socket_, endpoints_, ec);
		while(ec){
			asio::connect(socket_, endpoints_, ec);
		}
		write(inQueue, outQueue);
	}
	void write(std::queue<char *> &inQueue, std::queue<char *> &outQueue);	
};

/****************************************************
* ServerUnit is a class which initiates a server
* side connection. A ServerUnit READS from the socket
* and can be started without a ClientUnit.
****************************************************/
class ServerUnit{
public:
	//constructor
	ServerUnit(asio::io_service& io_service_, short port_, std::queue<char *> &inQueue, std::queue<char *> &outQueue)
	:acceptor_(io_service_, tcp::endpoint(tcp::v4(), port_)), socket_(io_service_){
		//if the client hangs up, the server cannot
		//reconnect ~ if we want the server to always
		//look for a connection put this function
		//call in a for loop
		start_accept(port_, inQueue); 
	}
	
private:
	//accept connection with client trying to merge on port
	void start_accept(short port_, std::queue<char *> &inQueue){
		std::cout << "Making connection on port " << port_ << std::endl;
		acceptor_.accept(socket_);
	
		std::cout << "reading..." << std::endl;
		for(;;){
			//connection closed by connected client
			if(start_reading(socket_, ec, inQueue) == EXIT_FAILURE){
				break;
			}

			
		}
		std::cout << "server done reading..." << std::endl;
		std::cout << "Stopped Reading... Disconnecting from port "<< port_ << std::endl;
		//close socket so we can search for new peer
		socket_.close();
	}

	//variables
	asio::error_code ec;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
};
