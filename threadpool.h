#pragma once
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include<vector>
#include<functional>
#include<queue>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<atomic>
#include<future>

namespace std{
    // 线程池最大容量，应该尽量设小一点
    #define THREADPOOL_MAX_NUM 16
    // 线程池，提交变参函数或lamda表达式的匿名函数执行，可以获取执行返回值

    class threadpool{
        unsigned short _initSize; //初始化线程数量
        using Task = function<void()>; //定义类型  可以认为是一个函数类型，接受任意原型是void()的函数
        vector<thread> _pool; // 线程池
        queue<Task> _tasks; // 任务队列
        mutex _lock; // 任务队列同步锁

        /*
            condition_variable _task_cv; 定义了一个条件变量 _task_cv。
            条件变量是多线程编程中的一种同步机制，用于线程间的等待和通知。它允许一个或多个线程等待某个条件的成立，而不需要主动轮询或忙等待，从而节省了系统资源。
            
            在这里，_task_cv 被用于线程池中的任务调度和执行。当线程池中的任务队列为空时，线程池的工作线程可以通过等待 _task_cv 来暂停自己的执行，以避免空闲时的资源浪费。
            而当有新的任务被添加到任务队列中时，通过通知 _task_cv，工作线程可以被唤醒并继续执行任务。

            条件变量的等待和通知操作需要与互斥锁（例如 std::mutex）配合使用，以确保线程之间的同步和互斥访问。
            在使用条件变量时，通常会有一个与之关联的互斥锁，以保护共享数据的访问和修改。
        */
        condition_variable _task_cv; // 条件阻塞

        // 定义了一个原子变量 _run，类型为 atomic<bool>，并初始化为 true。
        atomic<bool> _run{true}; // 线程池是否执行
        atomic<int> _idlThrNum{0}; // 空闲线程数量
    public:
        inline threadpool(unsigned short size = 4){
            _initSize = size;
            addThread(size);
        }

        inline ~threadpool(){
            _run = false;
            // 线程唤醒
            _task_cv.notify_all();

            for(thread& thread :_pool){
                // 等待任务结束之后退出
                if(thread.joinable())
                    thread.join();
            }
        }
    public:
        // 提交一个任务
        // 调用.get()获取返回值，会等待任务执行完，获取返回值
        /*
            两种方法调用类成员
            1. bind: .commit(std::bind(&Dog::sayHello, &dog));
            2. 使用mem_fn ： .commit(std::mem_fn(&Dog::sayHello), this);
        */
        template<class F, class...Args>
        auto commit(F&& f, Args&&... args) -> future<decltype(f(args...))>
        {
            if(!_run)
                throw runtime_error("commit on ThreadPool is stopped.");
            
            using RetType = decltype(f(args...)); // typename std::result_of<F(Args...)>::type, 函数 f 的返回值类型
            auto task = make_shared<packaged_task<RetType()>>(
                bind(forward<F>(f), forward<Args>(args)...)
            ); // 把函数入口及参数，打包(绑定)

            future<RetType> future = task->get_future();
            {
                // 添加任务到队列
                lock_guard<mutex> lock{_lock};
                _tasks.emplace([task](){(*task)();
                });
            }

            _tasks_cv.notify_one(); // 唤醒一个线程执行
            return future;
        }



        // 提交一个无参任务，且无返回值
        template<class F>
        void commit2(F&& task){
            if(!_run) return;
            {
                lock_guard<mutex> lock{_lock};
                _tasks.emplace(forward<F>(task));
            }


            _tasks_cv.notify_one(); // 唤醒一个线程执行
        }
        // 空闲线程数量
        int idlCount(){return _idlThrNum;}
        // 线程数量
        int thrCount(){return _pool.size();}

        void addThread(unsigned short size){
            for(;_pool.size() < THREADPOOL_MAX_NUM && size > 0; --size){
                // 增加线程数量，但是不超过预定义的数量 THREADPOOL_MAX_NUM
                _pool.emplace_back([this]{
                    while(true){
                        Task task;
                        {
                            unique_lock<mutex> lock{_lock};
                            _task_cv.wait(lock, [this]{
                                return !_run || !_tasks.empty();
                            });
                            if(!_run && _tasks.empty())
                                return;
                            _idlThrNum--;
                            task = move(_tasks.front());
                            _tasks.pop();
                        }
                        task(); // 执行任务

                        {
                            unique_lock<mutex> lock{_lock};
                            _idlThrNum++;
                
                        }
                            
                    }
                });
                {
                    unique_lock<mutex> lock{_lock};;
                    _idlThrNum++;
                }
            }
        }
    };
}

#endif