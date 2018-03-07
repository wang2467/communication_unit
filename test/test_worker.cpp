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
      cerr << "Usage: test_worker <local port> <host port> <host IP>\n";
      return EXIT_FAILURE;
    }

    vector<ConnectionInfo*> v;
    ConnectionInfo* info = (ConnectionInfo*)  malloc(sizeof(ConnectionInfo));
    info->localPort_ = argv[1];
    info->hostPort_ = argv[2];
    info->hostIP_ = argv[3];
    v.push_back(info);
    StartTransport* cu = new StartTransport(v);
    thread t(&StartTransport::start, cu);
    t.join();
    free(info);
    delete(cu);
  }
  catch (exception& e)
  {
    cerr << "Exception: " << e.what() << "\n";
  }

  return EXIT_SUCCESS;
}
