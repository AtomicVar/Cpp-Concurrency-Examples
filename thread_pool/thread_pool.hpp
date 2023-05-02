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
  task_queue<std::function<void()>> tasks_;
  std::vector<std::thread> threads_;

  // 每个工作线程：尝试从工作队列中取出一个任务，并执行
  void worker(int worker_id) {
    spdlog::info("worker {} started.", worker_id);
    std::function<void()> func;
    while (tasks_.pop(func)) {
      func();
    }
    spdlog::info("worker {} finished.", worker_id);
  }

 public:
  // 构造函数：创建 n_threads 个线程（worker）待命
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

  // 告诉任务队列准备关闭，然后等待所有线程结束
  void shutdown() {
    spdlog::info("Thread pool is shutting down...");
    tasks_.signal_for_kill();

    for (auto& t : threads_) {
      if (t.joinable()) {
        t.join();
      }
    }
  }

  // 提交一个函数，由线程池中的某一个线程（异步地）执行
  template <typename F, typename... Args>
  std::future<std::result_of_t<F(Args...)>> submit(F&& f, Args&&... args) {
    // 通过绑定 f 及其参数，得到一个无参数的函数 func
    auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

    // 用 std::packaged_task 封装 func，并将关联的 std::future 作为返回值；
    // 同时需要使用 shared_ptr 封装 task 得到 task_ptr：因为 std::packaged_task 不可拷贝，只能移动，
    // 无法继续被封装为 std::function<void()>
    auto task_ptr = std::make_shared<std::packaged_task<std::result_of_t<F(Args...)>()>>(func);
    auto ret = task_ptr->get_future();

    // 使用 Lambda 表达式将函数包装为 void() 类型
    std::function<void()> wrapper_func = [task_ptr]() { (*task_ptr)(); };

    // 将这个函数送入任务队列
    tasks_.push(wrapper_func);

    return ret;
  }

  void wait_for_all() {}

  ~ThreadPool() {
    shutdown();
  }
};