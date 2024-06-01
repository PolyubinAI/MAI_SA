### Without Cash
```
Running 5m test @ http://localhost:8080
  1 threads and 10 connections
  Thread Stats   Avg      Stdev       Max   +/- Stdev
    Latency   272.82 ms  445.01 ms   1.97s    82.95%
    Req/Sec    14.91      9.37       81.00     81.00%
  Latency Distribution
     50%   80.12 ms
     75%  273.54ms
     90%    1.01s 
     99%    1.82s 
  4756 requests in 5.00m, 0.95MB read
  Socket errors: connect 0, read 1390, write 0, timeout 531
  Non-2xx or 3xx responses: 2283
Requests/sec:     15.12
Transfer/sec:      3.25KB
```

### With Cash
```
Running 5m test @ http://localhost:8080
  1 threads and 10 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency   217.90ms  367.60ms   1.97s    86.93%
    Req/Sec    47.63     29.89     278.00     77.98%
  Latency Distribution
     50%   82.01ms
     75%  236.78ms
     90%  850.43ms
     99%    1.70s 
  13224 requests in 5.00m, 2.60MB read
  Socket errors: connect 0, read 3887, write 0, timeout 446
  Non-2xx or 3xx responses: 5609
Requests/sec:     44.69
Transfer/sec:      8.89KB
```
