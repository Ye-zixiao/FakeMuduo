//
// Created by Ye-zixiao on 2021/4/8.
//

#ifndef LIBFM_NET_BUFFER_H_
#define LIBFM_NET_BUFFER_H_

#include <algorithm>
#include <vector>
#include <cassert>
#include <string>

#include "libfm/base/Copyable.h"
#include "libfm/net/Endian.h"

namespace fm::net {

class Buffer : public Copyable {
 public:
  static constexpr size_t kCheapPrepend = 8;
  static constexpr size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize)
      : buffer_(kCheapPrepend + initialSize),
        read_index_(kCheapPrepend),
        write_index_(kCheapPrepend) {
    assert(readableBytes() == 0);
    assert(writableBytes() == initialSize);
    assert(prependableBytes() == kCheapPrepend);
  }

  // 是否有移动的必有？

  void swap(Buffer &rhs) noexcept {
    buffer_.swap(rhs.buffer_);
    std::swap(read_index_, rhs.read_index_);
    std::swap(write_index_, rhs.write_index_);
  }

  size_t readableBytes() const { return write_index_ - read_index_; }
  size_t writableBytes() const { return buffer_.size() - write_index_; }
  size_t prependableBytes() const { return read_index_; }

  const char *peek() const { return begin() + read_index_; }
  const char *findCRLF() const;
  const char *findCRLF(const char *start) const;
  const char *findEOL() const;
  const char *findEOL(const char *start) const;

  // 取走指定数量的数据，注意：retrieve取走和read/peek等数据获取操作是不相同！
  void retrieve(size_t len);
  void retrieveUntil(const char *end);
  void retrieveInt64() { retrieve(sizeof(int64_t)); }
  void retrieveInt32() { retrieve(sizeof(int32_t)); }
  void retrieveInt16() { retrieve(sizeof(int16_t)); }
  void retrieveInt8() { retrieve(sizeof(int8_t)); }
  void retrieveAll() { read_index_ = kCheapPrepend, write_index_ = kCheapPrepend; }

  std::string retrieveAllAsString() { return retrieveAsString(readableBytes()); }
  std::string retrieveAsString(size_t len);
  std::string toString() const { return std::string(peek(), readableBytes()); }
  std::string_view toStringView() const { return std::string_view(peek(), readableBytes()); }

  void append(const std::string &str) { append(str.data(), str.size()); }
  void append(const char *data, size_t len);
  void append(const void *data, size_t len) { append(static_cast<const char *>(data), len); }

  // 网络字节序的方式添加主机字节序数值
  void appendInt64(int64_t x);
  void appendInt32(int32_t x);
  void appendInt16(int16_t x);
  void appendInt8(int8_t x);

  // 从Buffer中取走指定字节大小的网络字节序数值，并转换为主机字节序
  int64_t readInt64();
  int32_t readInt32();
  int16_t readInt16();
  int8_t readint8();

  // 以网络字节序获取Buffer中的数据
  int64_t peekInt64() const;
  int32_t peekInt32() const;
  int16_t peekInt16() const;
  int8_t peekInt8() const;

  // 向Buffer中的前预留空间中写入指定字节大小的数据
  void prependInt64(int64_t x);
  void prependInt32(int32_t x);
  void prependInt16(int16_t x);
  void prependInt8(int8_t x);
  void prepend(const void *data, size_t len);

  size_t internalCapacity() const { return buffer_.capacity(); }
  const char *beginWrite() const { return buffer_.data() + write_index_; }
  char *beginWrite() { return buffer_.data() + write_index_; }
  void hasWritten(size_t len);
  void unwrite(size_t len);
  void shrink(size_t reserve);

  void ensureWritableBytes(size_t len);

  ssize_t readFd(int fd, int *savedErrno);

 private:
  char *begin() { return buffer_.data(); }
  const char *begin() const { return buffer_.data(); }

  void makeSpace(size_t len);

 private:
  static constexpr char kCRLF[] = "\r\n";

  std::vector<char> buffer_;
  size_t read_index_;
  size_t write_index_;
};

} // namespace fm::net

#endif //LIBFM_NET_BUFFER_H_
