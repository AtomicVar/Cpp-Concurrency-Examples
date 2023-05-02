#pragma once

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>
#include <iostream>

#include "task_queue.hpp"
#include "spdlog/spdlog.h"

class ThreadPool {
 private:
  TaskQueue<std::function<void()>> tasks_;
  std::vector<std::thread> threads_;

  /**
   * @brief 每个工作线程：尝试从工作队列中取出一个任务，并执行
   * 
   * @param worker_id 当前工作线程的 ID
   */
  void worker(int worker_id) {
    spdlog::info("worker {} started.", worker_id);
    std::function<void()> func;
    while (tasks_.pop(func)) {
      func();
    }
    spdlog::info("worker {} finished.", worker_id);
  }

 public:
  /**
   * @brief 构造函数：创建 n_threads 个线程（worker）待命
   * 
   * @param n_threads 线程数量，默认为机器 CPU 核数
   */
  ThreadPool(int n_threads = -1) {
    if (n_threads == -1) {
      n_threads = std::thread::hardware_concurrency();
    }
    for (int i = 0; i < n_threads; i++) {
      threads_.emplace_back(&ThreadPool::worker, this, i);
    }
  }

  // 禁止拷贝构造和移动构造
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&)      = delete;

  // 禁止拷贝赋值和移动赋值
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool& operator=(ThreadPool&&)      = delete;

  /**
   * @brief 告诉任务队列准备关闭，然后等待所有线程结束
   * 
   */
  void shutdown() {
    spdlog::info("Thread pool is shutting down...");
    tasks_.signal_for_kill();

    for (auto& t : threads_) {
      if (t.joinable()) {
        t.join();
      }
    }
  }

  /**
   * @brief 提交一个函数，由线程池中的某一个线程（异步地）执行
   * 
   * @param f 提交的函数（或任何可执行对象）
   * @param args 提交的函数（或任何可执行对象）的任意数量的参数
   * @return std::future<std::result_of_t<F(Args...)>> 
   */
  template <typename F, typename... Args>
  std::future<std::result_of_t<F(Args...)>> submit(F&& f, Args&&... args) {
    auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

    auto task_ptr = std::make_shared<std::packaged_task<std::result_of_t<F(Args...)>()>>(func);
    auto ret = task_ptr->get_future();

    std::function<void()> wrapper_func = [task_ptr]() { (*task_ptr)(); };

    tasks_.push(wrapper_func);

    return ret;
  }

  void wait_for_all() {}

  /**
   * @brief 析构函数：调用 shutdown() 通知所有线程退出
   * 
   */
  ~ThreadPool() {
    shutdown();
  }
};