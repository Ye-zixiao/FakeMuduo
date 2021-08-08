//
// Created by Ye-zixiao on 2021/6/7.
//

#ifndef LIBFM_HTTP_H_
#define LIBFM_HTTP_H_

#ifndef __linux__
#error "only support linux"
#else
#include "libfm/http/HttpRequest.h"
#include "libfm/http/HttpResponse.h"
#include "libfm/http/HttpServer.h"
#endif

#endif //LIBFM_HTTP_H_
