//
// Created by Ye-zixiao on 2021/4/8.
//

#include "Buffer.h"

#include <cstring>
#include <sys/uio.h>

#include "../base/Logging.h"
#include "SocketsOps.h"

using namespace fm;
using namespace fm::net;

const char *Buffer::findCRLF() const {
  const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
  return crlf == beginWrite() ? nullptr : crlf;
}

const char *Buffer::findCRLF(const char *start) const {
  assert(peek() <= start && start <= beginWrite());
  const char *crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
  return crlf == beginWrite() ? nullptr : crlf;
}

const char *Buffer::findEOL() const {
  const void *eol = memchr(peek(), '\n', readableBytes());
  return static_cast<const char *>(eol);
}

const char *Buffer::findEOL(const char *start) const {
  assert(peek() <= start && start <= beginWrite());
  const void *eol = memchr(start, '\n', beginWrite() - start);
  return static_cast<const char *>(eol);
}

void Buffer::retrieve(size_t len) {
  assert(len <= readableBytes());
  if (len < readableBytes())
	readIndex_ += len;
  else
	retrieveAll();
}

void Buffer::retrieveUntil(const char *end) {
  assert(peek() <= end && end <= beginWrite());
  retrieve(end - peek());
}

std::string Buffer::retrieveAsString(size_t len) {
  assert(len <= readableBytes());
  std::string result(peek(), len);
  retrieve(len);
  return result;
}

void Buffer::append(const char *data, size_t len) {
  ensureWritableBytes(len);
  std::copy(data, data + len, beginWrite());
  hasWritten(len);
}

void Buffer::appendInt64(int64_t x) {
  int64_t be64 = sockets::hostToNetwork64(x); // 无符号转换为有符号，可能存在点问题
  append(&be64, sizeof(be64));
}

void Buffer::appendInt32(int32_t x) {
  int32_t be32 = sockets::hostToNetwork32(x);
  append(&be32, sizeof(be32));
}

void Buffer::appendInt16(int16_t x) {
  int16_t be16 = sockets::hostToNetwork16(x);
  append(&be16, sizeof(be16));
}

void Buffer::appendInt8(int8_t x) {
  append(&x, sizeof(x));
}

int64_t Buffer::readInt64() {
  int64_t result = peekInt64();
  retrieveInt64();
  return result;
}

int32_t Buffer::readInt32() {
  int32_t result = peekInt32();
  retrieveInt32();
  return result;
}

int16_t Buffer::readInt16() {
  int16_t result = peekInt16();
  retrieveInt16();
  return result;
}

int8_t Buffer::readint8() {
  int8_t result = peekInt8();
  retrieveInt8();
  return result;
}

int64_t Buffer::peekInt64() const {
  assert(readableBytes() >= sizeof(int64_t));
  int64_t be64 = 0;
  memcpy(&be64, peek(), sizeof(be64));
  return sockets::networkToHost64(be64);
}

int32_t Buffer::peekInt32() const {
  assert(readableBytes() >= sizeof(int32_t));
  int32_t be32 = 0;
  memcpy(&be32, peek(), sizeof(be32));
  return sockets::hostToNetwork32(be32);
}

int16_t Buffer::peekInt16() const {
  assert(readableBytes() >= sizeof(int16_t));
  int32_t be16 = 0;
  memcpy(&be16, peek(), sizeof(be16));
  return sockets::hostToNetwork16(be16);
}

int8_t Buffer::peekInt8() const {
  assert(readableBytes() >= sizeof(int8_t));
  int8_t be8 = *peek();
  return be8;
}

void Buffer::prependInt64(int64_t x) {
  int64_t be64 = sockets::hostToNetwork64(x);
  prepend(&be64, sizeof(be64));
}

void Buffer::prependInt32(int32_t x) {
  int32_t be32 = sockets::hostToNetwork32(x);
  prepend(&be32, sizeof(be32));
}

void Buffer::prependInt16(int16_t x) {
  int16_t be16 = sockets::hostToNetwork16(x);
  prepend(&be16, sizeof(be16));
}

void Buffer::prependInt8(int8_t x) {
  prepend(&x, sizeof(x));
}

void Buffer::prepend(const void *data, size_t len) {
  assert(len <= prependableBytes());
  readIndex_ -= len;
  const char *d = static_cast<const char *>(data);
  std::copy(d, d + len, begin() + readIndex_);
}

void Buffer::hasWritten(size_t len) {
  assert(len <= writableBytes());
  writeIndex_ += len;
}

void Buffer::unwrite(size_t len) {
  assert(len <= readableBytes());
  writeIndex_ -= len;
}

void Buffer::shrink(size_t reserve) {
  Buffer other;
  other.ensureWritableBytes(readableBytes() + reserve);
  other.append(toString());
  swap(other);
}

void Buffer::ensureWritableBytes(size_t len) {
  if (writableBytes() < len)
	makeSpace(len);
  assert(writableBytes() >= len);
}

void Buffer::makeSpace(size_t len) {
  if (writableBytes() + prependableBytes() < len + kCheapPrepend)
	buffer_.resize(writeIndex_ + len);
  else {
	assert(kCheapPrepend < readIndex_);
	size_t readable = readableBytes();
	std::copy(begin() + readIndex_, begin() + writeIndex_,
			  begin() + kCheapPrepend);
	readIndex_ = kCheapPrepend;
	writeIndex_ = readIndex_ + readable;
	assert(readable == readableBytes());
  }
}

ssize_t Buffer::readFd(int fd, int *savedErrno) {
  char extraBuf[65536];
  struct iovec iov[2];
  const size_t writable = writableBytes();
  iov[0].iov_base = begin() + writeIndex_;
  iov[0].iov_len = writable;
  iov[1].iov_base = extraBuf;
  iov[1].iov_len = sizeof(extraBuf);

  // 缓冲区的上限就是64K bytes
  const int iovcnt = (writable < sizeof(extraBuf) ? 2 : 1);
  const ssize_t n = sockets::readv(fd, iov, iovcnt);
  if (n < 0)
	*savedErrno = errno;
  else if (writable <= static_cast<size_t>(writable))
	writeIndex_ += n;
  else {
	writeIndex_ = buffer_.size();
	append(extraBuf, n - writable);
  }
  return n;
}