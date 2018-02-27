//Script to run multithreaded manager

#include <ctime>
#include <iostream>
#include <string>
#include <mutex>
#include <queue>
#include <thread>
#include <ctime>
#include <chrono>
#include <cstdlib>
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
		
		std::thread t1 = std::thread (establishServer, atoi(localport_), std::ref(io_service_), std::ref(inQueue), std::ref(outQueue));
	
		std::thread t2 = std::thread (establishClient, std::ref(io_service_), host_, hostport_, std::ref(inQueue), std::ref(outQueue));
		
		t1.join();
		t2.join();

	}
};

void ClientUnit::write(std::queue<char *> &inQueue, std::queue<char *> &outQueue){
		{while(true){
			m.lock();
			//send data from outQueue to the worker	
			if(!outQueue.empty())  {
			send(outQueue, socket_, ec);
			outQueue.pop();
			}
			m.unlock();
		}}
}
void addFile(char * filename, std::queue<char *> &outQueue){
	FILE * fin = fopen(filename,"r");
	
	if(fin == NULL){
		std::cout << "fin = null" << std::endl;
	}
	
	fseek(fin, 0, SEEK_END);
	long len = ftell(fin);
	fseek(fin, 0, SEEK_SET);
	
	char *tst = (char *)malloc(sizeof(char)*(len+1));
	
	if(tst == NULL){
		std::cout << "malloc failed" << std::endl;
	}
	
	long dummy = fread(tst, 1, len, fin);
	tst[len] = '\0';
	outQueue.push(tst); 	
	fclose(fin);
}

static void establishManagerThread(asio::io_service& ios_, char *workerPort_, char *localPort_, char *IP_, std::queue<char *> &inQueue, std::queue<char *> &outQueue){
		CommUnit work(ios_, workerPort_, localPort_, IP_, std::ref(inQueue), std::ref(outQueue));
}
static void testQueue(std::queue<char *> &outQueue){
	int i = 0;
	for ( i = 0; i < 1000000; i++){
		char *tst = (char *)malloc(sizeof(char)*7);
		sprintf(tst, "%4d", i);
		m.lock();
		outQueue.push(tst);
		m.unlock();
	}
}
int main(int argc, char ** argv){
	try{
		if(argc != 1){
			std::cout << "Available Flags: s, c, sc " << std::endl;
			std::cout << "s for server, c for client, sc for server & client" << std::endl;

			std::cout << "Usage: <flag>" << std::endl;
			return 1;
		}


		static asio::io_service ios_;
		static char local1_[5] = {'9','9','9','2','\0'};
		static char local2_[5] = {'9','9','9','3','\0'};
		static char local3_[5] = {'9','9','9','4','\0'};
		static char worker1_[5] = {'3','1','1','2','\0'};
		static char worker2_[5] = {'4','1','1','2','\0'};
		static char worker3_[5] = {'5','1','1','2','\0'};
		
		static char IP_[10] = {'1','2','7','.','0','.','0','.','1','\0'};	
		std::queue<char *> inQueue;
		std::queue<char *> outQueue;
		
		std::thread t0 = std::thread (testQueue, std::ref(outQueue));
		std::thread t1 = std::thread (establishManagerThread, std::ref(ios_), worker1_, local1_, IP_, std::ref(inQueue), std::ref(outQueue));
	
		std::thread t2 = std::thread (establishManagerThread, std::ref(ios_), worker2_, local2_, IP_, std::ref(inQueue), std::ref(outQueue));
		std::thread t3 = std::thread (establishManagerThread, std::ref(ios_), worker3_, local3_, IP_, std::ref(inQueue), std::ref(outQueue));
		t0.join();
		t1.join();
		t2.join();
		t3.join();
	}

	catch(std::exception& e){
		std::cerr << e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}


/***********************************/
/******* THIS IS JUNK **************/
/***********************************/		
//outQueue.push(localport_);
		//outQueue.push(hostport_);
		//outQueue.push(IP_);
		/*char fi1[22] = "randomtextfiles/1.txt";
		fi1[21] = '\0';
		addFile(fi1, std::ref(outQueue));
		char fi2[22] = "randomtextfiles/2.txt";
		fi2[21] = '\0';
		addFile(fi2, std::ref(outQueue));
		char fi3[22] = "randomtextfiles/3.txt";
		fi3[21] = '\0';
		addFile(fi3, std::ref(outQueue));
		char fi4[22] = "randomtextfiles/4.txt";
		fi4[21] = '\0';
		addFile(fi4, std::ref(outQueue));
		char fi5[22] = "randomtextfiles/5.txt";
		fi5[21] = '\0';
		addFile(fi5, std::ref(outQueue));
		char fi6[22] = "randomtextfiles/6.txt";
		fi6[21] = '\0';
		addFile(fi6, std::ref(outQueue));
		char fi7[22] = "randomtextfiles/7.txt";
		fi7[21] = '\0';
		addFile(fi7, std::ref(outQueue));
		char fi8[22] = "randomtextfiles/8.txt";
		fi8[21] = '\0';
		addFile(fi8, std::ref(outQueue));
		char fi9[22] = "randomtextfiles/9.txt";
		fi9[21] = '\0';
		addFile(fi9, std::ref(outQueue));
		char fi10[23] = "randomtextfiles/10.txt";
		fi10[22] = '\0';
		addFile(fi10, std::ref(outQueue));
		char fi11[23] = "randomtextfiles/11.txt";
		fi11[22] = '\0';
		addFile(fi11, std::ref(outQueue));
		char fi12[23] = "randomtextfiles/12.txt";
		fi12[22] = '\0';
		addFile(fi12, std::ref(outQueue));
		char fi13[23] = "randomtextfiles/13.txt";
		fi13[22] = '\0';
		addFile(fi13, std::ref(outQueue));*/
	
