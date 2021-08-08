使用wrk性能实测结果如下：

$ wrk -t4 -c100 -d30s --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  4 threads and 100 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.14ms    2.17ms  52.43ms   90.83%
    Req/Sec    46.03k    10.29k   81.75k    67.83%
  Latency Distribution
     50%  296.00us
     75%    1.12ms
     90%    3.08ms
     99%   10.57ms
  5508782 requests in 30.10s, 404.53MB read
Requests/sec: 183029.47
Transfer/sec:     13.44MB