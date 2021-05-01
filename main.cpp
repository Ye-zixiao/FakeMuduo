// Author: Ye-zixiao
// Date: 2020-04-07

#include "base/ThreadPool.h"
#include "base/Logging.h"
#include "base/LogFile.h"
#include "base/TimeStamp.h"
#include "net/InetAddress.h"
#include "net/Socket.h"
#include "net/EventLoop.h"
#include "net/Channel.h"
#include "net/Buffer.h"
#include "net/TcpServer.h"
#include "http/HttpServer.h"
#include "http/HttpRequest.h"
#include "http/HttpResponse.h"

#include <functional>
#include <iostream>

using namespace std;
using namespace fm;
using namespace fm::net;
using namespace fm::net::http;

unordered_map<string, string> files;

void onRequest(const HttpRequest &request, HttpResponse *response) {
//  cout << "Headers " << request.methodToString() << " " << request.path() << endl;
//  for (const auto &header:request.headers()) {
//    cout << header.first << ": " << header.second << endl;
//  }
  string path = request.path();
  if (path.back() == '/') path.append("index.html");

  response->setVersion(request.getVersion());
  response->addHeader("Server", "FakeMuduo");

  if (request.getMethod() == HttpRequest::kGet &&
      files.find(path) != files.end()) {
    response->setStatusCode(HttpResponse::k200OK);
    response->setStatusMessage("OK");
    response->setBody(files[path]);
  } else {
    response->setStatusCode(HttpResponse::k404NotFound);
    response->setStatusMessage("Not Found");
    response->setCloseConnection(true);
  }
}

int main() {
  // 通过这种方式模仿文件缓存的方式，这样每一次我们就不需要为每一个请求重复性的打开文件
  // 这个部分也许可以改成自动打开，或者指定配置文件的方式来完成。
  files = {
      {"/index.html", ""},
      {"/xxx.jpg", ""},
      {"/favicon.ico", ""}
  };
  for (auto &elem:files)
    readSmallFile(ReadSmallFile::kBufferSize,
                  "root" + elem.first, &elem.second);

  EventLoop loop;
  InetAddress listenAddr(12000);
  HttpServer server(&loop, listenAddr, "HttpServer");
  server.setHttpCallback(onRequest);
  server.setThreadNum(8);
  server.start();
  loop.loop();

  return 0;
}