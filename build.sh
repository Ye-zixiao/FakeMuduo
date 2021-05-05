#! /bin/sh

mkdir -p build && \
  cd build && cmake .. -D CMAKE_BUILD_TYPE=Release \
  && cmake --build . --target all -j 8