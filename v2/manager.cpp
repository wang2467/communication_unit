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

	Sesssion(boost::asio::io_service& io_service): socket_(io_service), strand_(io_service), file_type(".png"), count(0){
		pid++;
	}

	void start(){
		boost::asio::async_read_until(socket_, header_buf, "\n\n", boost::bind(&Sesssion::handle_read_header, this,
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		 std::cout << socket_.remote_endpoint().port() << std::endl;
	}

	~Sesssion(){
		std::cout << "Disconnected" << std::endl;
	}

private:
	void handle_read_header(const boost::system::error_code& error, size_t bytes_transferred){
		if(! error){
			std::istream header_stream(&header_buf);
			header_stream >> file_size;
			header_stream.read(data_, 2);
			std::string file_name;
			file_name.assign(file_type);
			f.open(file_name.insert(0, std::to_string(count++)), std::fstream::out);
			temp_count = 0;
			f.seekg(0, std::ios::beg);
			pos_start = f.tellg();
			do{
				header_stream.read(data_, 1024);
				f.write(data_, header_stream.gcount());
			}while(header_stream.gcount() > 0);
			socket_.async_read_some(boost::asio::buffer(data_, 1024), boost::bind(&Sesssion::handle_read, this,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		} else{
			//delete this;
		}
	}

	void handle_read(const boost::system::error_code& error, size_t bytes_transferred){
		if(bytes_transferred > 0){
			printf("read %lu\n", bytes_transferred);
			f.write(data_, bytes_transferred);
			if ((f.tellg() - pos_start) >= (std::streampos)file_size){
				f.close();
				std::string ack = "ack";
				memset(data_, 0, 1024);
				strcpy(data_, ack.c_str());
				socket_.async_write_some(boost::asio::buffer(data_, 3), boost::bind(&Sesssion::handle_write, this,
					boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			} else{
					socket_.async_read_some(boost::asio::buffer(data_, 1024), boost::bind(&Sesssion::handle_read, this,
						boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			}
		}
	}

	void handle_write(const boost::system::error_code& error, size_t bytes_transferred){
		if (! error){
			boost::asio::async_read_until(socket_, header_buf, "\n\n", boost::bind(&Sesssion::handle_read_header, this,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		} else{
			delete this;
		}
	}


	std::string file_type;
	tcp::socket socket_;
	char data_[1024];
	boost::system::error_code error_;
	size_t bytes_transferred = 0;
	boost::asio::strand strand_;
	std::fstream f;
	static int pid;
	int count;
	size_t temp_count;
	std::streampos pos_start;

	boost::asio::streambuf header_buf;
	size_t file_size;
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
		start_accept();
	}

private:
	size_t thread_pool_size;
	tcp::acceptor acceptor_;
	boost::asio::io_service& io_service_;
};

int Sesssion::pid = 0;

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