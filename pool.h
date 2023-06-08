#pragma once
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include<vector>
#include<queue>
#include<atomic>
#include<future>

#include<stdexcept>


namespace std
{
    // 线程池最大容量，应尽量设小一点
#define THREADPOOL_MAX_NUM 16

    //线程池，可以提交变参函数或lamda表达式的匿名函数执行嗯，可以获取执行返回值
    //不直接支持类成员函数，支持类静态成员函数或全局函数，Opteron()函数等
    class threadpool{
        unsigned short _initSize; // 初始化线程数量
        using Task = function<void()>; //定义类型
        vector<thread> _pool; // 线程池
        queue<Task> _tasks; // 任务队列
        mutex _lock;  // 任务队列同步锁
#ifdef THREADPOOL_AUTO_GROW
        mutex _lockGrow;  // 线程池增长同步锁
#endif
        condition_variable _task_cv; // 条件阻塞
        atomic<bool> _run { true }; // 线程池是否执行 
        atomic<int> _idlThrNum{ 0 }; // 空闲线程数量

    public:
        // 构造函数，初始化县城池，设定初始线程数量
        inline threadpool(unsigned short size = 4){_initSize = size; addThread(size);}
        inline ~threadpool(){
            _run = false;
            _task_cv.notify_all(); // 唤醒所有线程执行
            for(thread& thread : _pool){
                if(thread.joinable())
                    thread.join(); // 等待任务结束，前提：线程一定会执行完
            }
        }
    public:
        //提交一个任务
        // 调用.get() 获取返回值。 会等待任务执行完，获取返回值
        // 两种方法实现调用类成员函数
        // 一种使用bind
        // 一种使用mem_fn：
        template<class F, class... Args>
        auto commit(F&& f, Args&&... args) -> future<decltype(f(args...))>
        {
            if(!_run) // 线程池已停止
                throw runtime_error("commit on ThreadPool is stopped.");
            using RetType = decltype(f(args...)); // 函数f的返回值类型
            auto task = make_shared<packaged_task<RetType()>>(
                bind(forward<F>(f), forward<Args>(args)...)
            ); // 把函数入口及参数打包（绑定）
            future<RetType> future = task->get_future();
            {
                lock_guard<mutex> lock{ _lock };
                _tasks.emplace([task()]{
                    (*task)();
                })-;
            }
#ifdef THREADPOOL_AUTO_GROW
            if(_idlThrNum < 1 && _pool.size() < THREADPOOL_MAX_NUM)
                addThread(1);
#endif // !threadpool_auto_grow
            _task_cv.notify_one(); // 唤醒一个线程执行

            return future;
        }

        // 提交一个无参任务，且无返回值
        template<class F>
        void commit2(F&& task){
            if(!_run) return;
            {
                lock_guard<mutex> lock{_lock};
                _task.emplace(std::forward<F>(task));
            }
#ifdef THREADPOOL_AUTO_GROW
            if(_idlThrNum < 1 && _pool.size() < THREADPOOL_MAX_NUM)
                addThread(1);
#endif
            _task_cv.notify_one();
        }

        // 空闲线程数量
        int idlCount(){return _idlThrNum;}
        // 线程数量
        int thrCount(){return _pool.size();}

#ifndef THREADPOOL_AUTO_GROW
private:
#endif

        void addThread(unsigned short size){
            #ifdef THREADPOOL_AUTO_GROW
                if(!run)
                    throw runtime_error("Grow on threadpool is stopped.");
                unique_lock<mutex> lockGrow{_lockGrow}; // 自动增长锁
            #endif
                for(;_pool.size() < THREADPOOL_MAX_NUM && size > 0; --size){
                    // 增加线程数量，但不超过预定义数量THREADPOOL_MAX_NUM
                    _pool.emplace_back([this]{
                        while(true){
                            Task task;
                            {
                                unique_lock<mutex> lock{_lock};
                                _task_cv.wait(lock,[this]{
                                    return !_run || !_tasks.empty();
                                });
                                if(!_run && _tasks.empty())
                                    return;
                                _idlThrNum--;
                                task = move(_tasks.front());
                                _tasks.pop();
                            }
                            task();
#ifdef THREADPOOL_AUTO_GROW
					        if (_idlThrNum>0 && _pool.size() > _initSize) //支持自动释放空闲线程,避免峰值过后大量空闲线程
						        return;
#endif // !THREADPOOL_AUTO_GROW
                            {
                                unique_lock<mutex> lock{_lock};
                                _idlThrNum++;
                            }

                        }
                    });
                    {
                        unique_lock<mutex> lock{_lock};
                        _idlThrNum++;
                    }
                }
        }


    };
}




#endif