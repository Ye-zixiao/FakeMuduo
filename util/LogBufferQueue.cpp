//
// Created by Ye-zixiao on 2021/8/7.
//

#include "util/LogBufferQueue.h"
#include <cassert>
#include "util/SpinLock.h"

namespace fm::log {

std::atomic<int> g_reuse_time{};
std::atomic<int> g_allocation_time{};
std::atomic<int> g_backup_push_time{};
std::atomic<int> g_backup_pop_time{};
std::atomic<int> g_backup_size{};

// 仅仅用来调试
void getAllocationForDebug(int &new_time, int &reuse_time,
                           int &backup_push_time, int &backup_pop_time,
                           int &backup_size) {
  new_time = g_allocation_time.load(std::memory_order_relaxed);
  reuse_time = g_reuse_time.load(std::memory_order_relaxed);
  backup_push_time = g_backup_push_time.load(std::memory_order_relaxed);
  backup_pop_time = g_backup_pop_time.load(std::memory_order_relaxed);
  backup_size = g_backup_size.load(std::memory_order_relaxed);
}

std::allocator<LogBuffer::Item> LogBuffer::allocator_;

LogBuffer::LogBuffer() :
    write_states_{},
    items_(allocator_.allocate(kSize + 1)) {
  // 构造之时只是分配了相应的内存空间
  static_assert(sizeof(Item) == 128, "sizeof Item != 128");
}

LogBuffer::~LogBuffer() {
  size_t write_count = this->write_states_[kSize].load();
  for (size_t i = 0; i < write_count; ++i)
    allocator_.destroy(&items_[i]);
  allocator_.deallocate(items_, kSize + 1);
}

bool LogBuffer::push(LogLine &&log_line, size_t write_index) {
  //   使用定位new在缓冲区中创建相应的日志行LogLine
  //  allocator_.construct(&items_[write_index], std::move(log_line));
  new(&items_[write_index]) Item(std::move(log_line));
  write_states_[write_index].store(1, std::memory_order_release);
  //  return write_states_[kSize].fetch_add(1, std::memory_order_relaxed) + 1 == kSize;// fix it
  if (write_states_[kSize].fetch_add(1, std::memory_order_relaxed) + 1 == kSize) {
    // 这里我觉得使用内存松弛序应该是没什么问题的
    // 由于这个LogBuffer需要复用，所以我们需要对其进行reset
    write_states_[kSize].store(0, std::memory_order_relaxed);
    return true;
  }
  return false;
}

bool LogBuffer::pop(LogLine &log_line, size_t read_index) {
  if (write_states_[read_index].load(std::memory_order_acquire)) {
    Item &item = items_[read_index];
    log_line = std::move(item.log_line);
    return true;
  }
  return false;
}

template<size_t SIZE>
RingQueue<SIZE>::RingQueue()
    :log_buffers_{},
     write_index_(0),
     read_index_(0),
     flag_() {

}

template<size_t SIZE>
bool RingQueue<SIZE>::push(std::unique_ptr<LogBuffer> &&log_buffer) {
  SpinLock spin_lock(flag_);
  size_t next_write_index = (write_index_ + 1) % (SIZE + 1);
  if (next_write_index == read_index_) // 环形队列满
    return false;
  log_buffers_[write_index_] = std::move(log_buffer);
  write_index_ = next_write_index;
#ifndef NDEBUG
  g_backup_push_time.fetch_add(1, std::memory_order_relaxed);
  g_backup_size.fetch_add(1, std::memory_order_relaxed);
#endif
  return true;
}

template<size_t SIZE>
bool RingQueue<SIZE>::pop(std::unique_ptr<LogBuffer> &log_buffer) {
  SpinLock spin_lock(flag_);
  if (write_index_ == read_index_) // 环形队列空
    return false;
  log_buffer = std::move(log_buffers_[read_index_]);
  read_index_ = (read_index_ + 1) % (SIZE + 1);
#ifndef NDEBUG
  g_backup_pop_time.fetch_add(1, std::memory_order_relaxed);
  g_backup_size.fetch_sub(1, std::memory_order_relaxed);
#endif
  return true;
}

LogBufferQueue::LogBufferQueue() :
    log_buffers_(),
    current_write_buffer_(nullptr),
    current_read_buffer_(nullptr),
    write_index_(0),
    read_index_(0),
    flag_{},
    backup_log_buffers_() {
  setupNextWriteLogBuffer();
}

/* 创建新的日志行缓冲区，并以独占指针进行管理，然后放入日志块缓冲队列中 */
void LogBufferQueue::setupNextWriteLogBuffer() {
  std::unique_ptr<LogBuffer> next_write_log_buffer;
  // 所需的日志行缓冲区空间最先会试图从环形空间缓冲区池中分配，只有当不足时才实时分配
  if (!backup_log_buffers_.pop(next_write_log_buffer)) {
    next_write_log_buffer = std::make_unique<LogBuffer>();
#ifndef NDEBUG
    g_allocation_time.fetch_add(1, std::memory_order_relaxed);
#endif
  } else {
#ifndef NDEBUG
    g_reuse_time.fetch_add(1, std::memory_order_relaxed);
#else
    ;
#endif
  }
  current_write_buffer_.store(next_write_log_buffer.get(), std::memory_order_release);

  SpinLock spin_lock(flag_);
  log_buffers_.push(std::move(next_write_log_buffer));
  write_index_.store(0, std::memory_order_relaxed); // 由自旋锁保证
}

LogBuffer *LogBufferQueue::getNextReadLogBuffer() {
  SpinLock spin_lock(flag_);
  return log_buffers_.empty() ? nullptr : log_buffers_.front().get();
}

void LogBufferQueue::push(LogLine &&log_line) {
  assert(this->current_write_buffer_.load(std::memory_order_relaxed));

  size_t write_index = write_index_.fetch_add(1, std::memory_order_relaxed);
  if (write_index < LogBuffer::kSize) {
    if (current_write_buffer_.load(std::memory_order_acquire)->
        push(std::move(log_line), write_index)) {
      // 如果当前的日志缓冲区LogBuffer满，则重新分配或从缓冲区空间池中重新利用旧的空间
      setupNextWriteLogBuffer();
    }
  } else {
    // 多个线程同时试图读当前写日志行缓冲区进行写入，但正好有一个线程写满了这个缓冲区，
    // 且同时新的写日志行缓冲区还没有分配完成，那么此时后续的线程自旋等待（这段时间较短）
    while (write_index_.load(std::memory_order_acquire) >= LogBuffer::kSize);
    push(std::move(log_line));
  }
}

bool LogBufferQueue::pop(LogLine &log_line) {
  if (!current_read_buffer_)
    current_read_buffer_ = getNextReadLogBuffer();

  LogBuffer *read_buffer = current_read_buffer_;
  if (!read_buffer) return false;

  if (read_buffer->pop(log_line, read_index_)) {
    // 日志行从读缓冲区中弹出成功
    ++read_index_;
    if (read_index_ == LogBuffer::kSize) {
      read_index_ = 0;
      current_read_buffer_ = nullptr;

      // 如果读缓冲区中所有的日志行都被读取，则将该读日志行缓冲区从队列中弹出，但这个
      // 缓冲区并不会立即释放掉，而是存放到环形空间缓存中进行保存，以备压入日志行时复用之
      std::unique_ptr<LogBuffer> up;
      {
        SpinLock spin_lock(flag_);
        up = std::move(log_buffers_.front());
        log_buffers_.pop();
      }
      backup_log_buffers_.push(std::move(up));
    }
    return true;
  }
  return false;
}

} // namespace fm::log