// Author: Ye-zixiao
// Date: 2020-04-07

#include <functional>
#include <string_view>
#include <iostream>
#include "libfm/base.h"
#include "libfm/net.h"
#include "libfm/http.h"

using namespace std;
using namespace fm;
using namespace fm::net;
using namespace fm::net::http;

unordered_map<string, string> files;

void onRequest(const HttpRequest &request, HttpResponse *resp) {
//  cout << "Headers " << request.methodToString() << " " << request.path() << endl;
//  for (const auto &header:request.headers()) {
//    cout << header.first << ": " << header.second << endl;
//  }

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
    fprintf(stderr, "usage: %s <ROOT_PATH>\n",
            basename(argv[0]));
    exit(EXIT_FAILURE);
  }

  Logger::setLogLevel(Logger::TRACE);

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

/* =================测试结果=================

$ wrk -t4 -c100 -d30s --latency http://localhost:12000/
Running 30s tests @ http://localhost:12000/
  4 threads and 100 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.79ms    4.15ms  60.06ms   89.30%
    Req/Sec    63.96k    15.38k  103.25k    63.10%
  Latency Distribution
     50%  201.00us
     75%  536.00us
     90%    6.48ms
     99%   19.39ms
  7631270 requests in 30.09s, 3.03GB read
Requests/sec: 253588.86
Transfer/sec:    103.02MB

 * =========================================
 * */

/**
新纪录：
 $ wrk -t4 -c100 -d30s --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  4 threads and 100 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.19ms    2.27ms  63.17ms   90.86%
    Req/Sec    44.67k    10.26k   86.62k    70.03%
  Latency Distribution
     50%  308.00us
     75%    1.16ms
     90%    3.21ms
     99%   11.06ms
  5336907 requests in 30.08s, 2.12GB read
Requests/sec: 177431.41
Transfer/sec:     72.08MB

*/