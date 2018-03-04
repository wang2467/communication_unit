#ifndef COMM_UNIT_HPP
#define COMM_UNIT_HPP

#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include <ctime>
#include "asio.hpp"
#include "queue.hpp"


struct ConnectionInfo{
	char *hostIP_;
	char *hostPort_;
	char *localPort_;
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
	Queue<char *> inQueue;
	Queue<char *> outQueue;
	std::vector<ConnectionInfo*> v;
	asio::io_service ios_;
	//std::thread t1;
	//std::thread t2;
	//std::thread t3;
	

	void establishCommunicationThread(asio::io_service& ios_, char *workerPort1_, char *local1_, char *workerIP1_);
	void start();
	StartTransport(std::vector<ConnectionInfo*> v);
	//StartTransport(char *local1_, char *local2_, char *local3_, char *workerport1_, char *workerport2_, char *workerport3_, char *workerIP1_, char *workerIP2_, char *workerIP3_);
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
	CommUnit(asio::io_service& io_service_, char *hostport_, char *localport_, char *host_, Queue<char *> &inQueue, Queue<char *> &outQueue);

	//begin establishing a local server connection
	static void establishServer(short port_, asio::io_service& ios_, Queue<char *> &inQueue, Queue<char*> &outQueue);

	//begin establishing a local client connection ~ keep trying to connect to a host server
	static void establishClient(asio::io_service& ios_, char *host_, char *port_, Queue<char *> &inQueue, Queue<char *> &outQueue);

	//space to start threads to establish a ServerUnit&ClientUnit
	void initializeCommUnit(asio::io_service& io_service_, char *hostport_, char *localport_, char *host_, Queue<char *> &inQueue, Queue<char *> &outQueue);
};


/*********************************************************
* The ClientUnit is a class which initiates a client side
* connection. Every ClientUnit depends on a ServerUnit to
* connect to. ClientUnit WRITES to the socket.
*********************************************************/

class ClientUnit{
public:

	//constructor
	ClientUnit(asio::io_service& io_service, char *host, char *port, Queue<char *> &inQueue, Queue<char *> &outQueue);
	
	//send a string to the socket
	void send();
	
	//build packet to send to socket
	void buildPacketToSend(char *message_, char *send_this_, char *header_);

	//build header for sending
	void buildHeader(char *message_, char *header_);
	
	//variables
	tcp::resolver resolver_;
	tcp::resolver::query query_;
	tcp::resolver::iterator endpoints_;
	tcp::socket socket_;
	asio::io_service& io_service_;
	asio::error_code ec;
	Queue<char *> &inQueue;
	Queue<char *> &outQueue;
};


/****************************************************
* ServerUnit is a class which initiates a server
* side connection. A ServerUnit READS from the socket
* and can be started without a ClientUnit.
****************************************************/

class ServerUnit{
public:
	//constructor
	ServerUnit(asio::io_service& io_service_, short port_, Queue<char *> &inQueue, Queue<char *> &outQueue);
	
	//accept connection with client trying to merge on port
	void accept();
	
	//continuously read from socket_
	int read(int i);
	
	//retrieve header of packet from socket
	void getHeader(char *header_);
	
	//retreve body (message) of packet from socket
	void getBody(char *header_, char *body_);

	//variables
	short port_;
	asio::error_code ec;
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	Queue<char*>& inQueue;
	Queue<char*>& outQueue;
};



#endif
