// Author: Ye-zixiao
// Date: 2020-04-07

#include "base/ThreadPool.h"
#include "base/Logging.h"
#include "base/LogFile.h"
#include "base/TimeStamp.h"

#include <iostream>

#include <unistd.h>

using namespace std;
using namespace fm;

LogFile g_logfile("fakeMuduo",1024*16);

void output(const char*msg,int len) {
  g_logfile.append(msg,len);
}

void flush() {
  g_logfile.flush();
}

void threadFunc() {
  usleep(100000);
  LOG_INFO<<"sub thread";
}

int main() {
  TimeStamp start(TimeStamp::now());

  Logger::setOutput(output);
  Logger::setFlush(flush);
  ThreadPool thread_pool("ThreadPool",1000);
  thread_pool.start(8);
  for(int i=0;i<1000;++i)
	thread_pool.run(threadFunc);
  thread_pool.stop();
  LOG_INFO<<"main thread end";

  TimeStamp end(TimeStamp::now());
  double diff=timeDiff(end,start);
  cout<<"threads waste "<<diff<<" seconds"<<endl;

  return 0;
}
