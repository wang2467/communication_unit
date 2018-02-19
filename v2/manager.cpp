#include <iostream>
#include <thread>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <vector>
#include <string>
#include <fstream>


using boost::asio::ip::tcp;

class Sesssion{
public:
	tcp::socket& getSocket(){
		return socket_;
	}

	Sesssion(boost::asio::io_service& io_service): socket_(io_service), strand_(io_service), f("cp.png", std::fstream::out){
	}

	void start(){
		 socket_.async_read_some(boost::asio::buffer(data_, 1024), boost::bind(&Sesssion::handle_read, this, 
		 	boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

private:

	// void handle_read(const boost::system::error_code& error, size_t bytes_transferred){
	// 	if (! error){
	// 		printf("%s\n", data_);
	// 		socket_.async_read_some(boost::asio::buffer(data_, 1024), boost::bind(&Sesssion::handle_read, this, 
	// 			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	// 	} else{
	// 		delete this;
	// 	}
	// }

	void handle_read(const boost::system::error_code& error, size_t bytes_transferred){
		if(error == boost::asio::error::eof){
			f.write(data_, bytes_transferred);
			f.close();
			socket_.async_read_some(boost::asio::buffer(data_, 1024), boost::bind(&Sesssion::handle_read, this, 
		 	boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		} else if(!error){
			f.write(data_, bytes_transferred);
			socket_.async_read_some(boost::asio::buffer(data_, 1024), boost::bind(&Sesssion::handle_read, this, 
		 	boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		} else{
			delete this;
		}
	}

	tcp::socket socket_;
	char data_[1024];
	boost::system::error_code error_;
	size_t bytes_transferred = 0;
	boost::asio::strand strand_;
	std::fstream f;
};

class CommunicationUnit{
public:
	CommunicationUnit(const size_t size, boost::asio::io_service& io_service, short port):
	thread_pool_size(size), io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), port)){
		start_accept();
	}; 

	void run(){
		std::vector<std::shared_ptr<std::thread> > threads;
		for (int i = 0; i < thread_pool_size; i++){
			std::shared_ptr<std::thread> thread(new std::thread([this](){
				io_service_.run();
			}));
			threads.push_back(thread);
		}
		for (int i = 0; i < thread_pool_size; i++){
			threads[i] -> join();
		}
	}

	void start_accept(){
		Sesssion* session = new Sesssion(io_service_);
		std::cout << "Got Connection" << std::endl;
		acceptor_.async_accept(session -> getSocket(), boost::bind(&CommunicationUnit::handle_accept, this, 
			boost::asio::placeholders::error, session));
	}

	void handle_accept(const boost::system::error_code& error, Sesssion* s){
		if (! error){
			s -> start();
		} else{
			delete this;
		}
	}

private:
	size_t thread_pool_size;
	tcp::acceptor acceptor_;
	boost::asio::io_service& io_service_;
};

int main(int argc, char* argv[]){
	try{
		if (argc != 2){
			std::cerr << "Usage: ./server <port>" << std::endl;
			return 1;
		}
		using namespace std;
		boost::asio::io_service io_service;
		CommunicationUnit c(1, io_service, atoi(argv[1]));
		c.run();
	} catch (std::exception& e){
		std::cerr << "Exception" << e.what() << std::endl;
	}
	return 0;
}