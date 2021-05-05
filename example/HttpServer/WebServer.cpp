// Author: Ye-zixiao
// Date: 2020-04-07

#include "../../src/FakeMuduo.h"

#include <functional>
#include <iostream>

using namespace std;
using namespace fm;
using namespace fm::net;
using namespace fm::net::http;

unordered_map<string, string> files;

void onRequest(const HttpRequest &request, HttpResponse *resp) {
  cout << "Headers " << request.methodToString() << " " << request.path() << endl;
  for (const auto &header:request.headers()) {
    cout << header.first << ": " << header.second << endl;
  }

  string path = request.path();
  if (path.back() == '/') path.append("index.html");

  resp->setVersion(request.getVersion());
  resp->addHeader("Server", "FakeMuduo");

  if (request.getMethod() == HttpRequest::kGet &&
      files.find(path) != files.end()) {
    resp->setStatusCode(HttpResponse::k200OK);
    resp->setStatusMessage("OK");
    resp->setBody(files[path]);
  } else {
    resp->setStatusCode(HttpResponse::k404NotFound);
    resp->setStatusMessage("Not Found");
    resp->setCloseConnection(true);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <ROOT_PATH>\n",
            basename(argv[0]));
    exit(EXIT_FAILURE);
  }

  files = {
      // 通过这种方式模仿文件缓存，这样每一次就不需要为每一个请求重复性的打开文件。
      // 当然该部分还可以改进，因为这种解决方案仅仅适合文件数少且小的情况
      {"/index.html", ""},
      {"/xxx.jpg", ""},
      {"/favicon.ico", ""}
  };
  for (auto &elem:files) {
    ReadSmallFile::readFile(string(argv[1]) + elem.first,
                            &elem.second, ReadSmallFile::k128KB);
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