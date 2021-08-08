HTTP服务器的基准性能压力测试推荐使用wrk：https://github.com/wg/wrk 。推荐博客：https://www.cnblogs.com/quanxiaoha/p/10661650.html 。

1、对于1000个用户的连接，开启一个main-Reactor线程，8个sub-Reactor线程，其结果如下：
>$ wrk -d30s -c1000 -t4 --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  4 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    13.32ms    5.03ms 209.41ms   97.46%
    Req/Sec    19.09k     1.25k   23.51k    89.00%
  Latency Distribution
     50%   12.84ms
     75%   14.07ms
     90%   15.42ms
     99%   23.53ms
  2279734 requests in 30.02s, 0.92GB read
Requests/sec:  75946.45
Transfer/sec:     31.29MB

测试结果显示：这1000个连接的大多数连接请求都可以在20ms的时间内得到响应，期间没有发生任何连接错误。

2、对于2500个用户的连接，开启一个main-Reactor线程，8个sub-Reactor线程，其结果如下：
>$ wrk -d30s -c2500 -t4 --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  4 threads and 2500 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    19.19ms    4.36ms 213.19ms   92.26%
    Req/Sec    13.11k     3.04k   19.64k    70.08%
  Latency Distribution
     50%   18.83ms
     75%   20.48ms
     90%   22.25ms
     99%   30.12ms
  1565835 requests in 30.01s, 645.10MB read
  Socket errors: connect 1483, read 148603, write 0, timeout 0
Requests/sec:  52168.44
Transfer/sec:     21.49MB

测试结果显示：这2500个连接的大多数连接请求大多数都在20ms的时间内得到响应，但期间出现了多个连接、读取错误。
                         可能是受到进程文件描述符有限的影响，因为2500-1483=1017≈1024。

3、对于5000个用户的连接，开启一个main-Reactor线程，8个sub-Reactor线程，其结果如下：
>$ wrk -d30s -c5000 -t4 --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  4 threads and 5000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    18.72ms    4.15ms 219.85ms   89.87%
    Req/Sec    13.42k     4.02k   19.43k    51.25%
  Latency Distribution
     50%   18.25ms
     75%   19.90ms
     90%   21.90ms
     99%   30.89ms
  1602002 requests in 30.01s, 660.00MB read
  Socket errors: connect 3983, read 152437, write 0, timeout 0
Requests/sec:  53375.79
Transfer/sec:     21.99MB

测试结果显示：这5000个连接的大多数连接请求大多数都在20ms的时间内得到响应，但期间出现了多个连接、读取错误。
                         可能是受到进程文件描述符有限的影响，因为5000-3983=1017≈1024。

现在解开可打开文件描述符上限，重新测试上面的内容。
1.1、对于1000个用户的连接，开启一个main-Reactor线程，8个sub-Reactor线程，其结果如下：
>$ wrk -d30s -t4 -c1000 --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  4 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    14.45ms    4.79ms 211.41ms   97.78%
    Req/Sec    17.56k     1.07k   19.17k    82.58%
  Latency Distribution
     50%   14.03ms
     75%   15.21ms
     90%   16.53ms
     99%   21.35ms
  2097163 requests in 30.01s, 864.00MB read
Requests/sec:  69887.86
Transfer/sec:     28.79MB

1.2、对于1000个用户的连接，开启一个main-Reactor线程，8个sub-Reactor线程，其结果如下：
>$ wrk -d30s -t12 -c1000 --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  12 threads and 1000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    14.33ms    5.31ms 223.04ms   98.48%
    Req/Sec     5.90k   450.85    14.03k    96.22%
  Latency Distribution
     50%   13.99ms
     75%   14.95ms
     90%   15.93ms
     99%   19.89ms
  2115673 requests in 30.10s, 0.85GB read
Requests/sec:  70288.91
Transfer/sec:     28.96MB

2.1、对于2500个用户的连接，开启一个main-Reactor线程，8个sub-Reactor线程，其结果如下：
>$ wrk -d30s -t4 -c2500 --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  4 threads and 2500 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    42.96ms   18.38ms 539.10ms   98.40%
    Req/Sec    14.95k   511.56    17.19k    83.08%
  Latency Distribution
     50%   41.73ms
     75%   43.46ms
     90%   45.16ms
     99%   59.08ms
  1785149 requests in 30.02s, 735.46MB read
Requests/sec:  59474.67
Transfer/sec:     24.50MB

2.2、对于2500个用户的连接，开启一个main-Reactor线程，8个sub-Reactor线程，其结果如下：
>$ wrk -d30s -t12 -c2500 --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  12 threads and 2500 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    43.09ms   19.91ms 562.71ms   98.32%
    Req/Sec     4.99k   472.11    14.03k    89.76%
  Latency Distribution
     50%   41.57ms
     75%   43.24ms
     90%   45.17ms
     99%   61.81ms
  1783214 requests in 30.02s, 734.66MB read
Requests/sec:  59393.63
Transfer/sec:     24.47MB

3.1、对于5000个用户的连接，开启一个main-Reactor线程，8个sub-Reactor线程，其结果如下：
>$ wrk -d30s -t4 -c5000 --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  4 threads and 5000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    86.44ms   40.63ms 966.22ms   97.40%
    Req/Sec    14.89k   753.78    18.63k    85.92%
  Latency Distribution
     50%   83.65ms
     75%   86.47ms
     90%   89.27ms
     99%  162.22ms
  1778101 requests in 30.04s, 732.56MB read
Requests/sec:  59197.99
Transfer/sec:     24.39MB

3.2、对于5000个用户的连接，开启一个main-Reactor线程，8个sub-Reactor线程，其结果如下：
>$ wrk -d30s -t12 -c5000 --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  12 threads and 5000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency    86.63ms   41.99ms 986.39ms   97.51%
    Req/Sec     4.97k   573.96    15.79k    89.65%
  Latency Distribution
     50%   83.50ms
     75%   86.36ms
     90%   89.52ms
     99%  173.62ms
  1774622 requests in 30.04s, 731.12MB read
Requests/sec:  59083.32
Transfer/sec:     24.34MB

4.1、对于10000个用户的连接，开启一个main-Reactor线程，8个sub-Reactor线程，其结果如下：
>$ wrk -d30s -t4 -c10000 --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  4 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   165.42ms   49.38ms   1.06s    93.99%
    Req/Sec    14.90k     1.38k   18.93k    70.67%
  Latency Distribution
     50%  166.16ms
     75%  170.55ms
     90%  174.90ms
     99%  300.17ms
  1779201 requests in 30.05s, 733.01MB read
Requests/sec:  59208.23
Transfer/sec:     24.39MB

4.2、对于10000个用户的连接，开启一个main-Reactor线程，8个sub-Reactor线程，其结果如下：
>$ wrk -d30s -t12 -c10000 --latency http://localhost:12000/
Running 30s test @ http://localhost:12000/
  12 threads and 10000 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   167.93ms   61.33ms   1.29s    94.67%
    Req/Sec     4.99k     1.51k   17.05k    73.70%
  Latency Distribution
     50%  166.38ms
     75%  170.74ms
     90%  175.14ms
     99%  391.34ms
  1776483 requests in 30.04s, 731.89MB read
  Socket errors: connect 0, read 1, write 0, timeout 0
Requests/sec:  59129.19
Transfer/sec:     24.36MB

从上面的测试结果可以看出来，使用FakeMuduo编写的http服务器承受10000个用户绝对是没有问题的，但为了承受着10000个用户，其所带来的延迟就要非常高，几乎90%的用户感受到的连接延迟达到了200ms。但用户这么多这也无可厚非。

总之：用户数越多，服务器的响应时间就越大，QPS也就越小。不过这个表明服务能力的QPS我个人感觉还是有一个最值点的，如上所示，1000个用户连接时的QPS达到7万完全不受问题。因此想要让服务器有一个好的效果，100ms以下的响应延迟，5w以上的QPS，推荐用在5000个客户的规模上。

当然上述的实验都是在同一台机器上做的（服务器、客户都是在同一台机子上的），性能评估可能并不一定准确，CPU为4790，4核8线程。还有待进一步的测试。

