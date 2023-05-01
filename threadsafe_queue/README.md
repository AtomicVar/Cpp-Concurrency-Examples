# threadsafe_queue

使用 `std::condition_variable` 实现的一个简单的线程安全的队列 `threadsafe_queue<T>`，支持以下接口：

1. `void push(const T& item)`
2. `void pop(T& ret)`
3. `bool try_pop(T& ret)`
4. `bool empty() const`

## 参考

- 《C++ 并发编程实战》（第二版）4.1.2