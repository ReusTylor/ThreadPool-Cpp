#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool {
public:
    ThreadPool(size_t);
    // 函数模板
    template<typename F, typename ... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;
    // 析构函数
    ~ThreadPool();
private:
    // need to keep track of threads so we can join them
    // 存储线程对象
    std::vector< std::thread > workers;

    // the task queue
    // 存储任务的队列，用于保存待执行的任务
    std::queue< std::function<void()> > tasks;
    
    // synchronization
    // 互斥锁
    std::mutex queue_mutex;
    // 条件变量用于实现进程同步
    std::condition_variable condition;
    // 布尔变量，指示线程池是否停止执行任务
    bool stop;
};
 
// the constructor just launches some amount of workers
// 根据给定的线程数创建一个线程池，并初始化成变量
inline ThreadPool::ThreadPool(size_t threads)
    :   stop(false)
{
    for(size_t i = 0;i<threads;++i)
        workers.emplace_back(
            // this捕获当前对象的指针
            [this]
            {
                // 无限循环 表示线程将一直运行，直到收到停止信号
                for(;;)
                {
                    // 声明一个可调用对象的变量，用于存储从任务队列中获取的任务
                    // task没有返回值，不带任何参数
                    std::function<void()> task;

                    {
                        // 创建一个独占的互斥锁，用于对任务队列的访问进行同步
                        std::unique_lock<std::mutex> lock(this->queue_mutex);

                        // 线程在此处等待，直到以下条件之一发生：
                        // 1. stop为true，表明线程池已经停止
                        // 2. 任务队列不为空,表示有任务可以执行
                        // wait函数用于阻塞当前线程，直到满足特定的条件才继续执行。当上面两种情况为真时，满足等待条件，继续执行
                        this->condition.wait(lock,
                            [this]{ return this->stop || !this->tasks.empty(); });
                        // 检查线程池是否已停止且任务队列为空，如果是，则结束当前线程的执行。
                        if(this->stop && this->tasks.empty())
                            return;
                        // 从任务队列中获取一个任务，并使用 std::move 进行移动语义的操作。
                        // 将任务队列中的第一个任务进行移动操作，将其所有权转移到 task 对象中。
                        task = std::move(this->tasks.front());
                        // 移除队列的头部元素，即已经获取的任务
                        this->tasks.pop();
                    }
                    // 执行获取到的任务，即调用可调用对象的 operator()。
                    task();
                }
            }
        );
}

// add new work item to the pool
// 函数模板，用于向任务队列中添加新的任务。
// 函数模板的目的是能够接受任意类型的可调用对象（如函数指针，函数对象，lamda表达式等）和对应的参数

// 函数模板定义，F和Args分别用于接受函数类型和函数参数类型
template<class F, class... Args>
// && 通用引用，可以接收左值和右值
auto ThreadPool::enqueue(F&& f, Args&&... args)
// 函数模板返回std::future对象，其中typename std::result_of<F(Args...)::type表示函数f的返回类型
// std::future用于异步操作的结果处理
    -> std::future<typename std::result_of<F(Args...)>::type>
{
    // using 关键字定义别名
    /*
     * std::result_of<F(Args...)>：类型转换工具，获取调用类型F传入参数类型Args后的返回值类型，换句话说，std::result_of<F(Args...)> 表达式返回了一个函数类型的返回值类型。
     * typename : 告诉编译器后面是一个类型而不是一个变量或者函数
     * 获取函数类型F 传入参数类型Args后的返回值类型
     */
    using return_type = typename std::result_of<F(Args...)>::type;

    // 创建了一个 std::packaged_task 对象，封装了需要执行的任务
    /*
     * std::forward<F>(f), std::forward<Args>(args)...完美转发，保持传入的参数类型不变
     * std::bind： 将函数类型和参数绑定，生成一个函数对象
     * std::packaged_task<return_type() ： 封装的函数没有参数，返回值类型为return_type。
     */
    // 这样task变成了可以在异步线程中执行的任务
    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
    
    // 获取与packaged_task关联的std::future对象，用于获取任务执行的结果
    // 将任务添加到线程池中的任务队列中
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);

        // don't allow enqueueing after stopping the pool
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        // 将任务封装成lamda函数，并存储在任务队列tasks中
        // 这里的task是一个智能指针，
        tasks.emplace([task](){ (*task)(); });
    }
    // 有新任务可用，唤醒
    condition.notify_one();
    // 返回之前获取的future对象，以便用户可以在需要时获取任务的执行结果
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
  // 循环遍历线程池中的所有工作线程，调用join函数等待他们完成，确保在析构函数执行完成之前，所有工作线程都已经推出，并且线程池中的资源都得到释放。
    for(std::thread &worker: workers)
        worker.join();
}

#endif