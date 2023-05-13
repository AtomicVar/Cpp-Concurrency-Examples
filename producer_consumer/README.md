# 生产者消费者模型

一般可以通过线程安全的**有限阻塞队列**这种数据结构来实现生产者消费者模型。

一个线程安全的有限阻塞队列拥有如下方法：

1. `BoundedBlockingQueue(int capacity)` 初始化队列，其中 `capacity` 代表队列长度上限。
2. `void enqueue(T element)` 在队首增加一个 `element`。如果队列满，调用线程被阻塞直到队列非满。
3. `T dequeue()` 返回队尾元素并从队列中将其删除。如果队列为空，调用线程被阻塞直到队列非空。
4. `int size()` 返回当前队列元素个数。

**生产者**调用 `enqueue` 方法将生产出来的数据放到队列中，**消费者**调用 `dequeue` 方法从队列中取出数据进行消费。

代码位于 [./bounded_blocking_queue.hpp](./bounded_blocking_queue.hpp)

## 参考

- [1188. 设计有限阻塞队列 - 力扣（LeetCode）](https://leetcode.cn/problems/design-bounded-blocking-queue/)