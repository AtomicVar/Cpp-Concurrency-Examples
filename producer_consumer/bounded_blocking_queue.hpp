#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class BoundedBlockingQueue {
  int capacity_;
  std::queue<T> data_;
  std::mutex mtx_;
  std::condition_variable cv_;

 public:
  BoundedBlockingQueue(int capacity) : capacity_(capacity) {}

  void enqueue(T element) {
    {
      std::unique_lock lk{mtx_};
      cv_.wait(lk, [this]() { return data_.size() < capacity_; });
      data_.push(element);
    }
    cv_.notify_one();
  }

  T dequeue() {
    T ret;
    {
      std::unique_lock lk{mtx_};
      cv_.wait(lk, [this]() { return !data_.empty(); });
      ret = data_.front();
      data_.pop();
    }
    cv_.notify_one();
    return ret;
  }

  int size() {
    std::unique_lock lk{mtx_};
    return data_.size();
  }
};
