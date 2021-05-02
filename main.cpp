// Author: Ye-zixiao
// Date: 2020-04-07

#include "FakeMuduo.h"

#include <functional>
#include <iostream>

using namespace std;
using namespace fm;
using namespace fm::net;
using namespace fm::net::http;

unordered_map<string, string> files;

void onRequest(const HttpRequest &request, HttpResponse *response) {
  cout << "Headers " << request.methodToString() << " " << request.path() << endl;
  for (const auto &header:request.headers()) {
    cout << header.first << ": " << header.second << endl;
  }

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

int main(int argc, char *argv[]) {
  files = {
      // 通过这种方式模仿文件缓存，这样每一次就不需要为每一个请求重复性的打开文件。
      // 当然该部分还可以改进，因为这种解决方案仅仅适合文件数少且小的情况
      {"/index.html", ""},
      {"/xxx.jpg", ""},
      {"/favicon.ico", ""}
  };
  for (auto &elem:files) {
    readSmallFile(ReadSmallFile::kBufferSize,
                  "root" + elem.first, &elem.second);
  }

  EventLoop loop;
  InetAddress listenAddr(12000);
  HttpServer server(&loop, listenAddr, "HttpServer");
  server.setHttpCallback(onRequest);
  server.setThreadNum(8, 0);
  server.start();
  loop.loop();

  return 0;
}