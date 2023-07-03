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

        }

    };
};




#endif