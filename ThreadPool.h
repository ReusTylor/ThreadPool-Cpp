/*
    管理一个任务队列，一个线程队列，然后每次取一个任务分配给一个线程去做，循环往复
*/
#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <vector>
#include<queue>
#include<memory>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<future>
#include<functional>
#include<stdexcept>

class ThreadPool{
public:
    ThreadPool(size_t);

    // 添加任务函数
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)->std::future<typename std::result_of<F(Args...)>::type>;
    ~ThreadPool();
private:
    std::vector<std::thread> workers; //线程数组
    std::queue<std::function<void() >> tasks; // 任务队列 ，存储了可调用对象，对象的类型是void

    //
    std::mutex queue_mutex; // 互斥量
    std::condition_variable condition; // 条件变量
    bool stop; // 是否停止
};

inline ThreadPool::ThreadPool(size_t threads):stop(false){
    for(size_t i = 0; i < threads; i++){
        // 依次调用线程处理
        workers.emplace_back([this]{
            for(;;){
                std::function<void()> task;{
                    // 对互斥量加锁
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    // 等待lamda函数返回值产生
                    // 如果没有任务 / stop了，则阻塞，互斥量解锁 => 释放给其他线程
                    // 有任务 / 没有stop 则不阻塞线程，互斥量加锁 => 处理任务
                    this->condition.wait(lock, [this]{return this->stop || !this->tasks.empty();});
                    // 没有处理任务 => 直接返回
                    if(this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}



#endif