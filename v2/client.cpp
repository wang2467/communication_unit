#include <iostream>
#include <string>
#include <fstream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main (int argc, char* argv[]){
	try{
		const char* port = "13";
		const unsigned int buffer_1024 = 65336;

		if (argc != 2){
			std::cout << "Usage: client <host>" << std::endl;
		}
		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query(argv[1], port);
		tcp::resolver::iterator endpoints = resolver.resolve(query);

		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoints);

		// std::fstream f(argv[2], std::fstream::out);
		// unsigned int count = 0;
		// while(1){
		// 	char buf[buffer_1024];
		// 	boost::system::error_code error;
		// 	1024_t len = socket.read_some(boost::asio::buffer(buf), error);
		// 	std::cout << "Read " << len << std::endl;
		// 	count+=len;
		// 	std::cout << "Read a total of " << count << sxtd::endl;
		// 	if (error == boost::asio::error::eof){
		// 		f.write(buf, len);
		// 		f.close();
		// 		break;
		// 	} else if (error){
		// 		throw boost::system::system_error(error);
		// 	} else{
		// 		f.write(buf, len);
		// 	}
		// }
		while(1){
			std::fstream file("myself.png");
			char* buff = new char[1024];
			unsigned int count = 0;
			std::cout << "sending" << std::endl;
			while (! file.eof()){
				memset(buff, 0, 1024);
				file.read(buff, 1024);
				boost::system::error_code ignored_error;
				unsigned int c = file.gcount();
				boost::asio::write(socket, boost::asio::buffer(buff, c), boost::asio::transfer_all(), ignored_error);
				count+=c;
			}
			file.close();
			delete[] buff;
			std::cout <<"Finished" <<std::endl;
			std::cout <<"Sent " << count <<" bytes" << std::endl;
			break;
		}

	} catch(std::exception& e){
		std::cout << "Excpetion" << e.what() << std::endl;
	}
	return 0;
}