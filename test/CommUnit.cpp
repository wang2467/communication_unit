#include <ctime>
#include <iostream>
#include <string>
#include <mutex>
#include <queue>
#include <thread>
#include <ctime>
#include "asio.hpp"
#include "queue.hpp"
#include "CommUnit.hpp"

//start transport funcs

StartTransport::StartTransport(std::vector<ConnectionInfo*> v)/*:ios_()*/{
	this -> v = v;
}

void StartTransport::start(){
	int i = 0;
	std::vector<std::shared_ptr<std::thread> > threads;
	for ( i = 0; i < v.size(); i++){
		std::shared_ptr<std::thread> t(new std::thread(&StartTransport::establishCommunicationThread, this, std::ref(ios_), v[i]->hostPort_, v[i]->localPort_, v[i]->hostIP_));
		threads.push_back(t);
	}
	//std::thread tx = std::thread(testQueue
	for (i = 0; i < threads.size(); i++){
		(threads[i]) -> join();
	}

}

void StartTransport::establishCommunicationThread(asio::io_service& ios_, char *workerPort_, char *localPort_, char *IP_){
	CommUnit work(ios_, workerPort_, localPort_, IP_, std::ref(inQueue), std::ref(outQueue));
}


//comm unit funcs
CommUnit::CommUnit(asio::io_service& io_service_, char *hostport_, char *localport_, char *host_, Queue<char *> &inQueue, Queue<char *> &outQueue){
	initializeCommUnit(io_service_, hostport_, localport_, host_, inQueue, outQueue);
}

void CommUnit::establishServer(short port_, asio::io_service& ios_, Queue<char *> &inQueue, Queue<char*> &outQueue){
	ServerUnit server(ios_, port_, inQueue, outQueue);
}

void CommUnit::establishClient(asio::io_service& ios_, char *host_, char *port_, Queue<char *> &inQueue, Queue<char *> &outQueue){
	printf("here\n");
	ClientUnit client(ios_, host_, port_, inQueue, outQueue);
	printf("end\n");

}

void CommUnit::initializeCommUnit(asio::io_service& io_service_, char *hostport_, char *localport_, char *host_, Queue<char *> &inQueue, Queue<char *> &outQueue){
		
	std::thread t1 = std::thread (establishServer, atoi(localport_), std::ref(io_service_), std::ref(inQueue), std::ref(outQueue));
	
	std::thread t2 = std::thread (establishClient, std::ref(io_service_), host_, hostport_, std::ref(inQueue), std::ref(outQueue));
		
	t1.join();
	t2.join();
}

//client unit funcs
ClientUnit::ClientUnit(asio::io_service& io_service, char *host, char *port, Queue<char *> &inQueue, Queue<char *> &outQueue)
: io_service_(io_service), socket_(io_service), resolver_(io_service), query_(host,port), ec(), inQueue(inQueue), outQueue(outQueue){
	printf("connect 1\n");
	endpoints_ = resolver_.resolve(query_);
	std::cout << host << "+" << port << std::endl;
	asio::connect(socket_, endpoints_, ec);
	while(ec){
		asio::connect(socket_, endpoints_, ec);
	}

	printf("connected\n");	
	for(;;){
		send();
	}
		
}

void ClientUnit::send(){
	char *msg_ = outQueue.pop();
	std::cout << msg_ << std::endl;
	char header_[7];
	buildHeader(msg_, header_);
	char send_this_[8 + (int)strlen(msg_)];
	buildPacketToSend(msg_, send_this_, header_);
	//send to socket
	asio::write(socket_, asio::buffer((std::string) send_this_),
	asio::transfer_all(), ec);
}
	
//build packet to send to socket
void ClientUnit::buildPacketToSend(char *message_, char *send_this_, char *header_){
	
	//build body	
	char body_[strlen(message_)];
	strcpy(body_, message_);
	
	//concatenate body with header
	strcpy(send_this_, header_);
	strcat(send_this_, body_);
	
	//set delimeter
	send_this_[7+strlen(message_)] = '\0';
}

//build header for sending
void ClientUnit::buildHeader(char *message_, char *header_){
	std::size_t body_length_ = std::strlen(message_);
	sprintf(header_, "%7d", static_cast<int>(body_length_));
}
	
//server unit funcs
ServerUnit::ServerUnit(asio::io_service& io_service_, short port_, Queue<char *> &inQueue, Queue<char *> &outQueue):acceptor_(io_service_, tcp::endpoint(tcp::v4(), port_)), socket_(io_service_),
inQueue(inQueue), outQueue(outQueue), port_(port_),ec(){
		//if the client hangs up, the server cannot
		//reconnect ~ if we want the server to always
		//look for a connection put this function
		//call in a for loop
		accept(); 
}

//accept connection with client trying to merge on port
void ServerUnit::accept(){
	std::cout << "Making connection on port " << port_ << std::endl;
	acceptor_.accept(socket_);
	for(int i = 0;;i++){
		//connection closed by connected client
		if(read(i) == EXIT_FAILURE){
			break;
		}
		
	}
	std::cout << "Connection Closed... disconnecting from port "<< port_ << std::endl;
	//close socket so we can search for new peer
	socket_.close();
}
	
//continuously read from socket_
int ServerUnit::read(int i){
	
	char *header_ = (char *)malloc(sizeof(char)*(8));
	getHeader(header_);
	//connection closed
	if(ec == asio::error::eof){//connection closed by peer
		std::cout << "ERROR: EOF REACHED: from header" << std::endl;
		return EXIT_FAILURE;
	}
	
	char *body_ = (char*)malloc(sizeof(char)*(atoi(header_) + 1));
	getBody(header_, body_);
	//connection closed
	if(ec == asio::error::eof){
		std::cout << "ERROR: EOF REACHED: for body" << std::endl;
		return EXIT_FAILURE;
	}
	
	//push recieved message to queue
	std::cout << "Message Recieved: " << sizeof(body_) << std::endl;
	char line[9];
	sprintf(line, "9%03d.png", i);
	FILE* fp = fopen(line, "r");
	if(fp == NULL)
	{
		std::cout << "fopen failed\n";
		std::cout << "filename = " << line << "\n";
		return EXIT_FAILURE;
	}
	fwrite(body_, sizeof(char), sizeof(body_), fp);
	fclose(fp);
	//inQueue.push(body_);
	return EXIT_SUCCESS;
}
	
//retrieve header of packet from socket
void ServerUnit::getHeader(char *header_){
	//read header of length 7 bytes from socket
	int bytes_read_ = socket_.read_some(asio::buffer(header_, 7), ec);
	
	//set delimeter on header_
	header_[7] = '\0';
	
	//check if we read wrong amount of bytes from socket
	if(bytes_read_ != 7 && bytes_read_ != 0){
		std::cout << "Incorrect number of bytes read when reading header" << std::endl;
	}
}
	
//retreve body (message) of packet from socket
void ServerUnit::getBody(char *header_, char *body_){
	
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
		std::cout << "ERROR: incorrect number of bytes read while reading body" <<std::endl;
	}
}

