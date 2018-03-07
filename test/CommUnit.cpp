#include <fstream>
#include <iostream>
#include <queue>
#include <thread>
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
	for (i = 0; i < threads.size(); i++){
		(threads[i]) -> join();
	}

}

void StartTransport::establishCommunicationThread(asio::io_service& ios_, char *workerPort_, char *localPort_, char *IP_){
	CommUnit work(ios_, workerPort_, localPort_, IP_, std::ref(inQueue), std::ref(outQueue));
}


//comm unit funcs
CommUnit::CommUnit(asio::io_service& io_service_, char *hostport_, char *localport_, char *host_, Queue<MessageInfo *> &inQueue, Queue<MessageInfo *> &outQueue){
	initializeCommUnit(io_service_, hostport_, localport_, host_, inQueue, outQueue);
}

void CommUnit::establishServer(short port_, asio::io_service& ios_, Queue<MessageInfo *> &inQueue, Queue<MessageInfo *> &outQueue){
	ServerUnit server(ios_, port_, inQueue, outQueue);
}

void CommUnit::establishClient(asio::io_service& ios_, char *host_, char *port_, Queue<MessageInfo *> &inQueue, Queue<MessageInfo *> &outQueue){
	ClientUnit client(ios_, host_, port_, inQueue, outQueue);
}

void CommUnit::initializeCommUnit(asio::io_service& io_service_, char *hostport_, char *localport_, char *host_, Queue<MessageInfo *> &inQueue, Queue<MessageInfo *> &outQueue){
		
	std::thread t1 = std::thread (establishServer, atoi(localport_), std::ref(io_service_), std::ref(inQueue), std::ref(outQueue));
	
	std::thread t2 = std::thread (establishClient, std::ref(io_service_), host_, hostport_, std::ref(inQueue), std::ref(outQueue));
		
	t1.join();
	t2.join();
}

//client unit funcs
ClientUnit::ClientUnit(asio::io_service& io_service, char *host, char *port, Queue<MessageInfo *> &inQueue, Queue<MessageInfo *> &outQueue)
: io_service_(io_service), socket_(io_service), resolver_(io_service), query_(host,port), ec(), inQueue(inQueue), outQueue(outQueue){
	endpoints_ = resolver_.resolve(query_);
	asio::connect(socket_, endpoints_, ec);
	while(ec){
		asio::connect(socket_, endpoints_, ec);
	}
	std::cout << "ClientUnit successfully connected to a ServerUnit" << std::endl;
	for(;;){
		send();
	}
		
}

void ClientUnit::send(){
	MessageInfo *msgInfo_ = (MessageInfo*) outQueue.pop();
	char *msg_ = msgInfo_ -> msg_;
	//build header
	char header_[header_length];
	buildHeader(msg_, header_, msgInfo_);

	//build full packet ~ header + message
	char send_this_[header_length + msgInfo_ -> size_ + 1];
	buildPacketToSend(msg_, send_this_, header_, msgInfo_);

	//send to socket
	std::cout << "Sending Message Size: " << sizeof(send_this_) << std::endl;
	asio::write(socket_, asio::buffer(send_this_, header_length + msgInfo_ -> size_), ec);
	
	free(msgInfo_ -> msg_);
	free(msgInfo_);
}
	
//build packet to send to socket
void ClientUnit::buildPacketToSend(char *msg_, char *send_this_, char *header_, MessageInfo* msgInfo_){
	
	//build body	
	//char body_[strlen(message_)]; (redundant)
	//strcpy(body_, message_); (redundant)
	
	//concatenate body with header
	std::memcpy(send_this_, header_, header_length);
	std::memcpy(send_this_ + header_length, msg_, msgInfo_ -> size_);
	send_this_[header_length + msgInfo_ -> size_] = '\0';
}

//build header for sending
void ClientUnit::buildHeader(char *message_, char *header_, MessageInfo * msgInfo_){
	std::memcpy(header_, &msgInfo_ -> size_, sizeof(msgInfo_ -> size_));
}
	
//server unit funcs
ServerUnit::ServerUnit(asio::io_service& io_service_, short port_, Queue<MessageInfo *> &inQueue, Queue<MessageInfo *> &outQueue):acceptor_(io_service_, tcp::endpoint(tcp::v4(), port_)), socket_(io_service_),
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
	MessageInfo* msgInfo_ = (MessageInfo *)malloc(sizeof(MessageInfo));
	if(msgInfo_ == NULL){
		std::cout << "Malloc failed in ServerUnit::read ~ msgInfo_" << std::endl;
		return EXIT_FAILURE;
	}
	char *header_ = (char *)malloc(sizeof(char)*header_length);
	if(header_ == NULL){
		std::cout << "Malloc failed in ServerUnit::read ~ header_" << std::endl;
		return EXIT_FAILURE;
	}
	getHeader(header_);
	//connection closed
	if(ec == asio::error::eof){//connection closed by peer
		std::cout << "ERROR: EOF REACHED: from header" << std::endl;
		return EXIT_FAILURE;
	}
	//insert message size
	std::memcpy(&msgInfo_ -> size_, header_, header_length);

	//get body of message aka main part
	char *body_ = (char*)malloc(sizeof(char)*(msgInfo_ -> size_ + 1));
	if(body_ == NULL){
		std::cout << "Malloc failed in ServerUnit::read ~ body_" << std::endl;
		return EXIT_FAILURE;
	}
	getBody(msgInfo_, body_);

	//connection closed
	if(ec == asio::error::eof){
		std::cout << "ERROR: EOF REACHED: for body" << std::endl;
		return EXIT_FAILURE;
	}
	
	//push recieved message to queue
	std::cout << "Received Message Size: " << sizeof(header_) + msgInfo_ -> size_ << std::endl;
	char line[9];
	sprintf(line, "9%03d.png", i);
	std::ofstream myfile (line, std::ofstream::binary);
	if(!myfile.is_open())
	{
		std::cout << "fopen failed\n";
		std::cout << "filename = " << line << "\n";
		return EXIT_FAILURE;
	}
	myfile.write(body_, msgInfo_ -> size_);
	myfile.close();
	//inQueue.push(body_);
	return EXIT_SUCCESS;
}
	
//retrieve header of packet from socket
void ServerUnit::getHeader(char *header_){
	//read header from socket
	int bytes_read_ = socket_.read_some(asio::buffer(header_, header_length), ec);

	//check if we read wrong amount of bytes from socket
	if(bytes_read_ != header_length && bytes_read_ != 0){
		std::cout << "Incorrect number of bytes read when reading header" << std::endl;
	}
}
	
//retreve body (message) of packet from socket
void ServerUnit::getBody(MessageInfo *msgInfo_, char *body_){
	
	//read the message from socket
	long bytes_read_ = asio::read(socket_, asio::buffer(body_, msgInfo_ -> size_));
	body_[msgInfo_ -> size_] = '\0';
	//check if we read wrong about of bytes from the socket
	if(bytes_read_ != msgInfo_ -> size_){
		std::cout << "ERROR: incorrect number of bytes read while reading body" <<std::endl;
	}
}

