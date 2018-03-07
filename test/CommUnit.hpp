#ifndef COMM_UNIT_HPP
#define COMM_UNIT_HPP

#include <iostream>
#include "asio.hpp"
#include "queue.hpp"


struct ConnectionInfo{
	char *hostIP_;
	char *hostPort_;
	char *localPort_;
};

struct MessageInfo{
	int ID_;
	long size_;
	char * msg_;
};

using asio::ip::tcp;
/*****************************************
* Establishes initial call to commUnit
* initialization. To change the number
* of desired commUnits to establish, 
* one must manually change in
* the constructors definition
*****************************************/
class StartTransport{
public:
	Queue<MessageInfo *> inQueue;
	Queue<MessageInfo *> outQueue;
	std::vector<ConnectionInfo*> v;
	asio::io_service ios_;	

	void establishCommunicationThread(asio::io_service& ios_, char *workerPort1_, char *local1_, char *workerIP1_);
	void start();
	StartTransport(std::vector<ConnectionInfo*> v);
};

/*******************************************************
* CommUnit establishes a local ServerUnit for a seperate 
* ClientUnit to connect to and begins
* attempting to pair a local ClientUnit with a seperate 
* host Server Unit. The ServerUnit and ClientUnit
* are being processed simultaneously by two threads
* allowing for simultaneous read and write operations
*******************************************************/
class CommUnit{
public:
	//constructor
	CommUnit(asio::io_service& io_service_, char *hostport_, char *localport_, char *host_, Queue<MessageInfo *> &inQueue, Queue<MessageInfo *> &outQueue);

	//begin establishing a local server connection
	static void establishServer(short port_, asio::io_service& ios_, Queue<MessageInfo *> &inQueue, Queue<MessageInfo *> &outQueue);

	//begin establishing a local client connection ~ keep trying to connect to a host server
	static void establishClient(asio::io_service& ios_, char *host_, char *port_, Queue<MessageInfo *> &inQueue, Queue<MessageInfo *> &outQueue);

	//space to start threads to establish a ServerUnit&ClientUnit
	void initializeCommUnit(asio::io_service& io_service_, char *hostport_, char *localport_, char *host_, Queue<MessageInfo *> &inQueue, Queue<MessageInfo *> &outQueue);
};


/*********************************************************
* The ClientUnit is a class which initiates a client side
* connection. Every ClientUnit depends on a ServerUnit to
* connect to. ClientUnit WRITES to the socket.
*********************************************************/

class ClientUnit{
public:

	//constructor
	ClientUnit(asio::io_service& io_service, char *host, char *port, Queue<MessageInfo *> &inQueue, Queue<MessageInfo *> &outQueue);
	
	//send a string to the socket
	void send();
	
	//build packet to send to socket
	void buildPacketToSend(char *message_, char *send_this_, char *header_, MessageInfo* msgInfo_);

	//build header for sending
	void buildHeader(char *message_, char *header_, MessageInfo* msgInfo_);
	
	//variables
	tcp::resolver resolver_;
	tcp::resolver::query query_;
	tcp::resolver::iterator endpoints_;
	tcp::socket socket_;
	asio::io_service& io_service_;
	asio::error_code ec;
	Queue<MessageInfo *> &inQueue;
	Queue<MessageInfo *> &outQueue;

	//header stores size of message
	//depends on data type of message size in message structure
	enum { header_length = sizeof(long) };
};


/****************************************************
* ServerUnit is a class which initiates a server
* side connection. A ServerUnit READS from the socket
* and can be started without a ClientUnit.
****************************************************/

class ServerUnit{
public:
	//constructor
	ServerUnit(asio::io_service& io_service_, short port_, Queue<MessageInfo *> &inQueue, Queue<MessageInfo *> &outQueue);
	
	//accept connection with client trying to merge on port
	void accept();
	
	//continuously read from socket_
	int read(int i);
	
	//retrieve header of packet from socket
	void getHeader(char *header_);
	
	//retreve body (message) of packet from socket
	void getBody(MessageInfo * msgInfo_, char *body_);

	//variables
	short port_;
	asio::error_code ec;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	Queue<MessageInfo *>& inQueue;
	Queue<MessageInfo *>& outQueue;

	//header stores size of message
	//depends on data type of message size in message structure
	enum { header_length = sizeof(long) };
};



#endif
