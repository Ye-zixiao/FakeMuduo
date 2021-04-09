// Author: Ye-zixiao
// Date: 2020-04-07

#include "base/ThreadPool.h"
#include "base/Logging.h"
#include "base/LogFile.h"
#include "base/TimeStamp.h"
#include "net/InetAddress.h"
#include "net/Socket.h"

#include <unistd.h>

#include <iostream>

using namespace std;
using namespace fm;
using namespace fm::net;

int main() {
  InetAddress address("192.168.1.235",32);
  cout<<address.toIpPortStr()<<endl;
  cout<<address.toIpStr()<<endl;

  InetAddress address1("fe80::215:5dff:fee3:2896",13,true);
  cout<<address1.toIpPortStr()<<endl;
  cout<<address1.toIpStr()<<endl;

  Socket socket1(sockets::createNonblockingSocketOrDie(AF_INET));
  socket1.bindAddress(InetAddress(12000,false,false));
  socket1.listen();

  InetAddress client;
  int clientfd=socket1.accept(&client);
  cout<<client.toIpPortStr()<<endl;
  sockets::close(clientfd);

  return 0;
}