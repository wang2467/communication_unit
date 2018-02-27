#include <ctime>
#include <iostream>
#include <string>
#include <mutex>
#include <queue>
#include <thread>
#include <ctime>
#include <chrono>
#include "asio.hpp"
#include "commUnit.hpp"

using asio::ip::tcp;

class CommUnit{
public:
	CommUnit(asio::io_service& io_service_, char *hostport_, char *localport_, char *host_, std::queue<char *> &inQueue, std::queue<char *> &outQueue){
		initializeWorker(io_service_, hostport_, localport_, host_, inQueue, outQueue);
	}
private:
	
	static void establishServer(short port_, asio::io_service& ios_, std::queue<char *> &inQueue, std::queue<char*> &outQueue){
		ServerUnit server(ios_, port_, inQueue, outQueue);
	
	}
	static void establishClient(asio::io_service& ios_, char *host_, char *port_, std::queue<char *> &inQueue, std::queue<char *> &outQueue){
		ClientUnit client(ios_, host_, port_, inQueue, outQueue);
	} 
	
	void initializeWorker(asio::io_service& io_service_, char *hostport_, char *localport_, char *host_, std::queue<char *> &inQueue, std::queue<char *> &outQueue){
		
		std::thread t2 = std::thread (establishClient, std::ref(io_service_), host_, hostport_, std::ref(inQueue), std::ref(outQueue));
		std::thread t1 = std::thread (establishServer, atoi(localport_), std::ref(io_service_), std::ref(inQueue), std::ref(outQueue));
			
		t1.join();
		t2.join();

	}
};
void ClientUnit::write(std::queue<char *> &inQueue, std::queue<char *> &outQueue){
	{while(true){
		//pushing whatever receive from the manager back in queue to send back to the manager
		m.lock();
		if(!inQueue.empty()){
		char *temp = inQueue.front();
		outQueue.push(temp);
		inQueue.pop();
		}
		m.unlock();

		m.lock();
		if(!outQueue.empty()){
	//	std::cout << "Sending: " << outQueue.front() << std::endl;
		send(outQueue, socket_, ec);
		char *temp = outQueue.front();
		outQueue.pop();
		free(temp);	
		}
		m.unlock();

	}}
}
static void establishWorkerThread(asio::io_service& ios_, char *hostport_, char *localport_, char *IP_, std::queue<char *> &inQueue, std::queue<char *> &outQueue){
		CommUnit work(ios_, hostport_, localport_, IP_, std::ref(inQueue), std::ref(outQueue));
}
int main(int argc, char ** argv){
	try{
		if(argc != 1){
			std::cout << "Available Flags: s, c, sc " << std::endl;
			std::cout << "s for server, c for client, sc for server & client" << std::endl;

			std::cout << "Usage: <flag>" << std::endl;
			return 1;
		}
		asio::io_service ios_;
		char hostport1_[5] = {'9','9','9','2'};
		char hostport2_[5] = {'9','9','9','3'};
		char hostport3_[5] = {'9','9','9','4'};
		
		char worker1_[5] = {'3','1','1','2','\0'};
		char worker2_[5] = {'4','1','1','2','\0'};
		char worker3_[5] = {'5','1','1','2','\0'};

		
		char IP_[10] = {'1','2','7','.','0','.','0','.','1','\0'};	
		//inQueue is the data being recieved through the server
		std::queue<char *> inQueue_worker1_;
		std::queue<char *> inQueue_worker2_;
		std::queue<char *> inQueue_worker3_;
		
		//outQueue is the data ready to be sent back to the manager
		std::queue<char *> outQueue_worker1_;
		std::queue<char *> outQueue_worker2_; 
		std::queue<char *> outQueue_worker3_; 
		
		std::thread t1 = std::thread (establishWorkerThread, std::ref(ios_), hostport1_, worker1_, IP_, std::ref(inQueue_worker1_), std::ref(outQueue_worker1_));
	
	
		std::thread t2 = std::thread (establishWorkerThread, std::ref(ios_), hostport2_, worker2_, IP_, std::ref(inQueue_worker2_), std::ref(outQueue_worker2_));
		
		std::thread t3 = std::thread (establishWorkerThread, std::ref(ios_), hostport3_, worker3_, IP_, std::ref(inQueue_worker3_), std::ref(outQueue_worker3_));
	
		t1.join();
		t2.join();
                t3.join();
	}
	catch(std::exception& e){
		std::cerr << e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}
