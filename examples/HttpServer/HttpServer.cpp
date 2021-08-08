//
// Created by Ye-zixiao on 2021/8/8.
//

#include <iostream>
#include <libgen.h>
#include "libfm/fmnet/http/HttpServer.h"
#include "libfm/fmnet/http/HttpRequest.h"
#include "libfm/fmnet/http/HttpResponse.h"
#include "libfm/fmnet/EventLoop.h"
#include "libfm/fmutil/Log.h"
#include "ReadSmallFile.h"

using namespace std;
using namespace fm::net;
using namespace fm::net::http;

unordered_map<string, string> files;

void onRequest(const HttpRequest &request, HttpResponse *resp) {
  string path(request.path());
  if (path.back() == '/') path.append("index.html");

  resp->setVersion(request.getVersion());
  resp->addHeader("Server", "Libfm");

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
    fprintf(stderr, "usage: %s <ROOT_PATH>\n", basename(argv[0]));
    exit(EXIT_FAILURE);
  }

  fm::log::setLogLevel(fm::log::LogLevel::kDEBUG);
//  fm::log::initialize(fm::log::AsyncLoggerTag{}, "./", "HttpServer", 512 * 1024 * 1024);
  fm::log::initialize(fm::log::StdLoggerTag{});

  files = {
      // 通过这种方式模仿文件缓存，这样每一次就不需要为每一个请求重复性的打开文件。
      // 当然该部分还可以改进，因为这种解决方案仅仅适合文件数少且小的情况
      {"/index.html", ""},
      {"/xxx.jpg", ""},
      {"/favicon.ico", ""}
  };
  for (auto &elem:files) {
    examples::ReadSmallFile::readFile(string(argv[1]) + elem.first,
                                      &elem.second, examples::ReadSmallFile::k128KB);
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