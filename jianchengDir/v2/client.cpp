#include <iostream>
#include <string>
#include <fstream>
#include <boost/asio.hpp>
#include <cstring>

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

		int ct = 0;
		while(ct < 6){
			std::fstream file;
			boost::system::error_code ignored_error;
			if (ct == 0){
				file.open("myself.png", std::fstream::in);
			} else if (ct == 1){
				file.open("test1.png", std::fstream::in);
			} else if (ct == 2){
				file.open("test2.png", std::fstream::in);
			} else if (ct == 3){
				file.open("test3.png", std::fstream::in);
			} else if (ct == 4){
				file.open("test4.png", std::fstream::in);
			} else if (ct == 5){
				file.open("test5.png", std::fstream::in);
			}
			std::streampos start = file.tellg();
			file.seekg(0, std::ios::end);
			size_t file_size = file.tellg() - start;
			std::cout << "file size " << file_size << std::endl;
			file.seekg(0, std::ios::beg);
			boost::asio::streambuf header_buf;
			std::ostream header_stream(&header_buf);
			header_stream << file_size << "\n\n";
			boost::asio::write(socket, header_buf, boost::asio::transfer_all(), ignored_error);
			ct++;
			char* buff = new char[1024];
			unsigned int count = 0;
			std::cout << "sending" << std::endl;
			while (! file.eof()){
				memset(buff, 0, 1024);
				file.read(buff, 1024);
				unsigned int c = file.gcount();
				boost::asio::write(socket, boost::asio::buffer(buff, c), boost::asio::transfer_all(), ignored_error);
				count+=c;
			}
			file.close();
			delete[] buff;
			std::cout <<"Finished" <<std::endl;
			std::cout <<"Sent " << count <<" bytes" << std::endl;
			size_t len = socket.read_some(boost::asio::buffer(buff, 1024));
			std::string rec(buff);
			std::string ack(rec.substr(0, 3));
			if (strcmp(ack.c_str(), "ack") != 0){
				break;
			}
		}

	} catch(std::exception& e){
		std::cout << "Excpetion" << e.what() << std::endl;
	}
	return 0;
}