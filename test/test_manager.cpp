#include <cstdlib>
#include <iostream>
#include <queue>
#include <thread>
#include "asio.hpp"
#include "CommUnit.hpp"
#include "queue.hpp"

using asio::ip::tcp;
using namespace std;

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 4)
    {
      cerr << "Usage: test_manager <local port> <host port> <host IP>\n";
      return EXIT_FAILURE;
    }

    vector<ConnectionInfo*> v;
    ConnectionInfo* info = (ConnectionInfo*) malloc(sizeof(ConnectionInfo));
    info->localPort_ = argv[1];
    info->hostPort_ = argv[2];
    info->hostIP_ = argv[3];
    v.push_back(info);
    StartTransport* cu;
    thread t(&StartTransport::start, cu);
    char line[9];
    char* buf;
    for(int i = 0; i < 30; i++)
    {
      sprintf(line, "9%03d.png", i);
      FILE* fp = fopen(line, "r");
      if(fp == NULL)
      {
        cout << "fopen failed\n";
        cout << "filename = " << line << "\n";
        break;
      }
      fseek(fp, 0, SEEK_END);
      unsigned int filesize = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      fread(buf, sizeof(char), filesize, fp);
      fclose(fp);
      cu->outQueue.push(buf);
    }
    t.join();
    free(info);
  }
  catch (exception& e)
  {
    cerr << "Exception: " << e.what() << "\n";
  }
  return EXIT_SUCCESS;
}
