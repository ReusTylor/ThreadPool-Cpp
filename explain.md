### thread
首先，thread.joinable() 是一个成员函数，用于判断线程是否可以被加入（join）。只有在线程处于活动状态（即已启动但尚未完成）时，调用 joinable() 返回 true。

接下来，thread.join() 是一个成员函数，用于等待线程的完成。调用 join() 将阻塞当前线程，直到被调用的线程执行完成。如果线程已经执行完成，那么调用 join() 将立即返回。


因此，这两行代码的作用是首先检查线程是否可以被加入，即线程是否处于活动状态。如果是活动状态，则调用 join() 来等待线程执行完成，以确保在主线程继续执行之前，子线程的任务已经完成。

这样做的目的是为了避免主线程提前退出，导致子线程还未完成工作。通过调用 join()，主线程将等待子线程完成后再继续执行，确保线程同步和正确的执行顺序。



### 原子类型
atomic 是 C++ 中的原子类型，用于在多线程环境下对共享数据进行原子操作，以保证操作的原子性和线程安全性。原子类型提供了一些特殊的成员函数和操作符，以确保对变量的读取、写入和修改是原子的，不会受到其他线程的干扰。

atomic<bool> 表示一个原子布尔类型的变量，可以通过原子操作来读取和修改它的值。原子类型的变量操作可以保证线程间的同步和一致性，避免了竞态条件和数据竞争问题。


代码中的 _run{true} 则是对 _run 变量进行初始化，将其初始值设为 true。在后续的代码中，可以使用原子操作对 _run 进行读取和修改，而不需要额外的同步机制（如互斥锁）。


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



### 函数模板
`template<class F, class...Args>`

template 关键字表示接下来是一个模板定义。
<class F, class... Args> 是模板参数列表，用于声明模板参数。在这个例子中，有两个模板参数：F 和 Args。F 表示函数类型，Args 表示一系列的参数类型。class... 是 C++11 中的语法，表示模板参数包，允许接受任意数量的模板参数。
在函数模板定义中，可以使用这些模板参数来定义函数的参数、返回类型和函数体

`auto commit(F&& f, Args&&... args) -> future<decltype(f(args...))>`

auto 关键字表示函数的返回类型将根据表达式 f(args...) 的类型自动推导得出。这样可以确保返回类型与调用表达式的结果类型一致。
commit 是函数的名称。
(F&& f, Args&&... args) 是函数的参数列表。其中，F&& f 表示一个右值引用类型的参数 f，Args&&... args 表示一系列右值引用类型的参数 args，使用模板参数包的形式允许接受任意数量的参数。
-> future<decltype(f(args...))> 是函数的返回类型。future 表示一个异步任务的结果，decltype(f(args...)) 用于推导 f(args...) 表达式的类型，并将其作为 future 的模板参数。

### lamda函数
lambda函数的语法是[] () { ... }
```C++
// 捕获当前对象的指针
[this]
// 这里()为空，表示函数参数列表为空
{
    for(;;)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->condition.wait(lock,
                [this]{ return this->stop || !this->tasks.empty(); });
            if(this->stop && this->tasks.empty())
                return;
            task = std::move(this->tasks.front());
            this->tasks.pop();
        }

        task();
    }
}

```




### 移动语义



### using关键字定义别名




### 智能指针
1. unique_ptr<T> 独占资源所有权的指针
2. shared_ptr<T> 共享资源所有权的指针
3. weak_ptr<T> 共享资源的观察者，需要和std::shared_ptr 一起使用，不影响资源的生命周期。
   
#### std::unique_ptr
当独占资源的所有权时，可以使用std::unique_ptr对资源进行管理——离开 unique_ptr 对象的作用域时，会自动释放资源。这是很基本的 RAII 思想。

1. 使用裸指针，要记得释放内存
   ```C++
   {
    int* p = new int(100);
    // ... 
    delete p;
   }
   ```
2. 使用unique_ptr 自动管理内存
    ```C++
    {
        std::unique_ptr<int> uptr = std::make_unique(200);
    }

    ```
3. move-only
4. 指向一个数组
    ```C++
    {
        std::unique_ptr<int[]> uptr = std::make_unique<int[]>(10);
        for (int i = 0; i < 10; i++) {
            uptr[i] = i * i;
        }   
        for (int i = 0; i < 10; i++) {
            std::cout << uptr[i] << std::endl;
        }   
    }
    ```



#### shared_ptr
    对资源作引用计数--当计数为0时，自动释放资源
### std::result_of()
用于获取给定函数类型或函数对象的返回类型
### std::bind()



### std::forward()


### std::future()
C++11中的一种异步操作结果的表示对象，提供了一种机制来访问异步操作的结果或者等待异步操作的完成

std::future 对象表示一个异步操作的未来结果，它可以与异步任务关联，以获取该任务的结果。通过 std::future 对象，可以进行以下操作：
 1. 等待异步操作完成：使用 std::future::wait 或 std::future::wait_for 函数等待异步操作的完成。
 2. 获取异步操作的结果：使用 std::future::get 函数获取异步操作的结果，该函数会阻塞当前线程，直到异步操作完成并返回结果。
 3. 检查异步操作的状态：使用 std::future::valid 函数检查 std::future 对象是否与有效的异步操作关联。
 4. 同步获取异步操作的结果：使用 std::future::share 函数将 std::future 对象转换为 std::shared_future 对象，以允许多个线程共享异步操作的结果。

通过 std::future，可以方便地进行异步操作的管理和结果获取，从而实现异步编程和并发控制。


### RAII原理