//
// Created by Ye-zixiao on 2021/6/6.
//

#ifndef LIBFM_LIBFM_BASE_H_
#define LIBFM_LIBFM_BASE_H_

#ifndef __linux__
#error "only support linux"
#else
#include "base/BoundBlockingQueue.h"
#include "base/CountDownLatch.h"
#include "base/Exception.h"
#include "docs/oldfiles/base/FileUtil.h"
#include "docs/oldfiles/base/Logging.h"
#include "docs/oldfiles/base/LogFile.h"
#include "docs/oldfiles/base/SystemClock.h"
#include "docs/oldfiles/base/Timestamp.h"
#include "docs/oldfiles/base/ThreadPool.h"
#endif

#endif //LIBFM_LIBFM_BASE_H_
