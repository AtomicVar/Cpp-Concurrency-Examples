# spinlock

使用 `std::atomic_flag` 实现一个最简单的自旋锁 `spinlock_mutex`，支持以下两个方法：

1. `lock()`
2. `unlock()`

代码位于 [./spinlock.hpp](./spinlock.hpp)。

## Memory Order 的设置

TODO

## 参考

- 《C++ 并发编程实战》（第二版）5.2.2