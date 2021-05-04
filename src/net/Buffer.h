//
// Created by Ye-zixiao on 2021/4/8.
//

#ifndef FAKEMUDUO_NET_BUFFER_H_
#define FAKEMUDUO_NET_BUFFER_H_

#include <algorithm>
#include <vector>
#include <cassert>
#include <string>

#include "../base/copyable.h"
#include "Endian.h"

namespace fm {
namespace net {

class Buffer : public copyable {
 public:
  static constexpr size_t kCheapPrepend = 8;
  static constexpr size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize)
	  : buffer_(kCheapPrepend + initialSize),
		readIndex_(kCheapPrepend),
		writeIndex_(kCheapPrepend) {
	assert(readableBytes() == 0);
	assert(writableBytes() == initialSize);
	assert(prependableBytes() == kCheapPrepend);
  }

  // 是否有移动的必有？

  void swap(Buffer &rhs) {
	buffer_.swap(rhs.buffer_);
	std::swap(readIndex_, rhs.readIndex_);
	std::swap(writeIndex_, rhs.writeIndex_);
  }

  size_t readableBytes() const { return writeIndex_ - readIndex_; }
  size_t writableBytes() const { return buffer_.size() - writeIndex_; }
  size_t prependableBytes() const { return readIndex_; }

  const char *peek() const { return begin() + readIndex_; }
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
  void retrieveAll() { readIndex_ = kCheapPrepend, writeIndex_ = kCheapPrepend; }

  std::string retrieveAllAsString() { return retrieveAsString(readableBytes()); }
  std::string retrieveAsString(size_t len);
  std::string toString() const { return std::string(peek(), readableBytes()); }

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
  const char *beginWrite() const { return buffer_.data() + writeIndex_; }
  char *beginWrite() { return buffer_.data() + writeIndex_; }
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
  size_t readIndex_;
  size_t writeIndex_;
};

} // namespace net
} // namespace fm

#endif //FAKEMUDUO_NET_BUFFER_H_
