#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Session{
public:
	Session(boost::asio::io_service& io_service):socket_(io_service){
		pid_++;
	}

	tcp::socket& getSocket(){
		return socket_;
	}

	void start(){
		socket_.async_read_some(boost::asio::buffer(data_, max_length), boost::bind(&Session::handle_read, this, 
			boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

	int getPid(){
		return pid_;
	}

	~Session(){
		pid_--;
		std::cout << "Disconnected: " << pid_ << std::endl;
	}

private:
	tcp::socket socket_;
	enum 
	{
		max_length = 1024
	};
	char data_[max_length];
	static int pid_;

	void handle_read(const boost::system::error_code& error, size_t bytes_transferred){
		if (!error){
			strcat(data_, " --CAM2");
			size_t len = bytes_transferred+7;
			socket_.async_write_some(boost::asio::buffer(data_, len), boost::bind(&Session::handle_write, this,
				boost::asio::placeholders::error));
		} else{
			delete this;
		}
	}

	void handle_write(const boost::system::error_code& error){
		if (!error){
			std::cout << "Writing..." << std::endl;
			socket_.async_read_some(boost::asio::buffer(data_, max_length), boost::bind(&Session::handle_read, this,
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
		} else{
			delete this;
		}
	}

};

class CommunicationUnit{
public:
	CommunicationUnit(boost::asio::io_service& io_service, short port): io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), port)){
		start_accept();
	}


private:
	void start_accept(){
		Session* session_ = new Session(io_service_);
		std::cout << "Got Connection with PID: " << session_ -> getPid() << std::endl;
		acceptor_.async_accept(session_->getSocket(), 
			boost::bind(&CommunicationUnit::handle_accept, this, session_, boost::asio::placeholders::error));
	}

	void handle_accept(Session* session_, const boost::system::error_code& error){
		if (!error){
			session_ -> start();
		} else{
			delete session_;
		}
		start_accept();
	}

	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
};

int Session::pid_ = 0;


int main(int argc, char* argv[]){
	try{
		if (argc != 2){
			std::cerr << "Usage: ./server <port>" << std::endl;
			return 1;
		}
		using namespace std;
		boost::asio::io_service io_service;
		CommunicationUnit c(io_service, atoi(argv[1]));
		io_service.run();
	} catch (std::exception& e){
		std::cerr << "Exception" << e.what() << std::endl;
	}
	return 0;
}