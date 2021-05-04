#! /bin/sh

mkdir build && \
  cmake .. -H. -Bbuild -D CMAKE_BUILD_TYPE=Release \
  && cmake --build . --target all