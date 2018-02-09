#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>
#include <tuple>

using boost::asio::ip::tcp;

class CommunicationUnitClient{
private:
	tcp::resolver resolver_;
	tcp::resolver::query query_;
	tcp::resolver::iterator endpoints_;
	tcp::socket socket_;
	boost::asio::io_service& io_service_;
	char message_[1024];
public:
	CommunicationUnitClient(boost::asio::io_service& io_service, char* host, char* port): 
	io_service_(io_service), socket_(io_service), resolver_(io_service), query_(host, port){
		endpoints_ = resolver_.resolve(query_);
		boost::asio::connect(socket_, endpoints_);
	}

	void send(const char* request){
		strcpy(message_, request);
		size_t request_len = strlen(message_);
		socket_.write_some(boost::asio::buffer(message_, request_len));
	}

	std::tuple<char*, size_t> receive(){
		size_t response_len = socket_.read_some(boost::asio::buffer(message_));
		return std::make_tuple(message_, response_len);
	}
};



int main(int argc, char* argv[]){
	if (argc != 3){
		std::cerr << "Usage: ./client <host> <port>" << std::endl;
		return 1;
	}
	try{
		char message[1024];
		size_t response_len;
		char* res;
		for (;;){
			boost::asio::io_service io_service;
			CommunicationUnitClient c(io_service, argv[1], argv[2]);
			std::cout << "Enter Message: ";
			std::cin.getline(message, sizeof(message)/sizeof(char));
			c.send(message);
			std::tie(res, response_len)= c.receive();
			std::cout << res << std::endl;
			std::cout << response_len << std::endl;
		}

	}catch (std::exception& e){
		std::cout << "Error:" << e.what() << std::endl;
	}
	return 0;
}

