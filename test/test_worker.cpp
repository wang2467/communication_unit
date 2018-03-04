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
/*    if (argc != 4)
    {
      cerr << "Usage: test_worker <local port> <host port> <host IP>\n";
      return EXIT_FAILURE;
    }*/

    vector<ConnectionInfo*> v;
    ConnectionInfo* info = (ConnectionInfo*) malloc(sizeof(ConnectionInfo));
    char localPort[5] = {'2', '2', '2', '2', '\0'};
    char hostPort[5] = {'1', '1', '1', '1', '\0'};
    char hostIP[10] = {'1', '2', '7', '.', '0', '.', '0', '.', '1', '\0'};
    info->localPort_ = localPort;
    info->hostPort_ = hostPort;
    info->hostIP_ = hostIP;
    v.push_back(info);
    StartTransport* cu;
    thread t(&StartTransport::start, cu);
    t.join();
    free(info);
  }
  catch (exception& e)
  {
    cerr << "Exception: " << e.what() << "\n";
  }

  return EXIT_SUCCESS;
}
