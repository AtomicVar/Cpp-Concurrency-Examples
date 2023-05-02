# ThreadPool

使用 [`std::packaged_task`](https://en.cppreference.com/w/cpp/thread/packaged_task) 和 [`std::future`](https://en.cppreference.com/w/cpp/thread/future) 实现一个简单的线程池 `ThreadPool`，代码包含两个文件：

1. [thread_pool.hpp](./thread_pool.hpp)：线程池 class `ThreadPool` 的实现。
2. [task_queue.hpp](./task_queue.hpp)：任务队列 class `TaskQueue` 的实现。

## 1. 线程池设计

线程池 `ThreadPool` 主要提供以下接口：

1. `submit`：向线程池**提交**一个任务（函数）。
    ```c++
    template <typename F, typename... Args>
    std::future<std::result_of_t<F(Args...)>>
    submit(F&& f, Args&&... args)
    ```
    - `submit` 的参数 `f` 是任意类型的函数（或任何可调用对象），`args` 是用于调用 `f` 的任意数量、任意类型的参数。
    - `submit` 返回一个 `std::future` 对象，用户可以通过调用这个 `std::future` 的 `.get()` 方法来得到 `f` 的返回值。

线程池 `ThreadPool` 拥有以下私有成员变量：
```c++
TaskQueue<std::function<void()>> tasks_;  // 任务队列，线程们会从这里取任务执行（如果有的话）
std::vector<std::thread> threads_;         // 线程池里的线程们
```

### submit

`submit` 的实现只需短短几行：

```c++
template <typename F, typename... Args>
std::future<std::result_of_t<F(Args...)>> submit(F&& f, Args&&... args) {
  auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

  auto task_ptr = std::make_shared<std::packaged_task<std::result_of_t<F(Args...)>()>>(func);
  auto ret = task_ptr->get_future();

  std::function<void()> wrapper_func = [task_ptr]() { (*task_ptr)(); };

  tasks_.push(wrapper_func);

  return ret;
}
```

解释：
1. 通过绑定 `f` 及其参数，得到一个无参数的函数 `func`：
    ```c++
    auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    ```
    无参数的 `func` 会在后面更容易处理。
2. 用 `std::packaged_task` 封装 `func`，并将关联的 `std::future` 作为返回值：
    ```c++
    auto task_ptr = std::make_shared<std::packaged_task<std::result_of_t<F(Args...)>()>>(func);
    auto ret = task_ptr->get_future();
    ```
3. 使用 Lambda 表达式将函数包装为 `void()` 类型的 `std::function`：
    ```c++
    std::function<void()> wrapper_func = [task_ptr]() { (*task_ptr)(); };
    ```
4. 将这个函数送入任务队列：
    ```c++
    tasks_.push(wrapper_func);
    ```
5. 返回 `ret`。

**为什么需要使用 `shared_ptr` 封装 `task`？**

首先，`std::packaged_task` 不可拷贝，只能移动。如果要在后面第 3 步将其传入 Lambda 表达式，则必须使用 C++14 引入的 [Generalized Capture](https://learn.microsoft.com/en-us/cpp/cpp/lambda-expressions-in-cpp?view=msvc-170#generalized-capture-c-14)：
```c++
std::function<void()> wrapper_func = [task = std::move(task)]() mutable { task(); };
```
同时，由于 [`std::packaged_task::operator()`](https://en.cppreference.com/w/cpp/thread/packaged_task/operator()) 只能在非 const 的对象上调用，所以这里要加一个 `mutable` 关键字。

但这样仍然不行，因为 `std::function` [要求从一个可拷贝的对象上构造](https://en.cppreference.com/w/cpp/utility/functional/function/function)，但现在这个 Lambda 表达式由于捕获了一个 `std::packaged_task`，这个是不可拷贝的，所以整个 Lambda 表达式也不可拷贝了。

所以解决方法就是用智能指针把 `std::packaged_task` 包装起来，使之能够被拷贝。

这个问题也可以参考：[StackOverflow: Error on moving a packaged_task object to lambda capture](https://stackoverflow.com/questions/36958929/error-on-moving-a-packaged-task-object-to-lambda-capture)。

### 工作线程的工作内容

`ThreadPool` 还有一个私有的成员函数 `worker`，它就是线程池里每个线程要做的事：
```c++
void worker(int worker_id) {
  std::function<void()> func;
  while (tasks_.pop(func)) {
    func();
  }
}
```
其工作原理非常简单：尝试从任务队列中取任务，如果没有任务，就阻塞着；有任务，就拿出来执行掉，等待下一个任务。如果队列即将关闭，则退出 `while` 循环，当前线程结束。

### 构造与析构

`ThreadPool` 的构造函数负责创建线程，析构函数负责通知线程关闭：

```c++
ThreadPool(int n_threads = -1) {
  if (n_threads == -1) {
    n_threads = std::thread::hardware_concurrency();
  }
  for (int i = 0; i < n_threads; i++) {
    threads_.emplace_back(&ThreadPool::worker, this, i);
  }
}

~ThreadPool() {
  shutdown();
}
```

## 2. 任务队列设计

任务队列和本仓库的另一个线程安全队列 [`ThreadSafeQueue`](../ThreadSafeQueue/) 基本一致，其主要接口如下：

1. 推送任务到队列：
    ```c++
    template <typename T>
    void TaskQueue<T>::push(const T& item)
    ```
2. 从队列中取任务，如果队列中无任务，就阻塞，直到取到任务，返回 `true`；若队列准备关闭，就立刻返回 `false`：
    ```c++
    template <typename T>
    bool TaskQueue<T>::pop(T& ret)
    ```
3. 通知队列即将关闭：
    ```c++
    template <typename T>
    void TaskQueue<T>::signal_for_kill()
    ```

## 3. 参考

- [mtrebi/thread-pool](https://github.com/mtrebi/thread-pool)
- 《C++ 并发编程实战》（第二版）9.1 节
- [apache/mxnet](https://github.com/apache/mxnet)