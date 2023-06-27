### std::condition_variable 
用于线程之间的同步和通信。提供了一种机制，使得一个或者多个线程可以等待某个特定条件的发生，当条件满足时，其他线程可以通过通知来唤醒等待的线程
`std::condition_variable` 在多线程编程中通常与 `std::mutex` 和 `std::unique_lock` 一起使用。

主要用法
1. 创建对象
   ```C++
    std::condition_variable condition;
   ```
2. 等待条件发生
    ```C++
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock,[/*condition*/]{/*返回条件是否满足的布尔值*/})
    ```
    这会使当前进程阻塞，直到满足指定的条件。
    
    `condition.wait()` 接受一个 `std::unique_lock` 对象和一个 lambda 表达式作为参数。lambda 表达式用于指定等待条件，返回值为一个布尔值，表示条件是否满足。
3. 唤醒等待的线程
    ```C++
    condition.notify_one();  // 唤醒一个等待中的线程
    condition.notify_all();  // 唤醒所有等待中的线程
    ```
    在线程池的代码中，`std::condition_variable` 被用于线程的同步。当任务队列为空时，线程会调用 `condition.wait()` 进入阻塞状态，等待新任务的到来。当有新任务被添加到队列时，通过 `condition.notify_one()` 唤醒一个等待中的线程，使其开始执行任务。