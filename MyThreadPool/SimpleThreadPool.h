//
// Created by guessever on 23-8-5.
//

#pragma once

#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <thread>
#include <functional>
#include <future>


template<typename T>
class SafeQueue{
 public:
  SafeQueue() = default;
  ~SafeQueue() = default;
  SafeQueue(const SafeQueue& other) = delete;
  SafeQueue& operator=(const SafeQueue& other) = delete;
  SafeQueue(SafeQueue&& other) = delete;
  SafeQueue& operator=(const SafeQueue&& other) = delete;
  SafeQueue(const SafeQueue&& other) = delete;

  bool empty(){
    std::unique_lock<std::mutex> lock(m_Mutex);
    return m_Queue.empty();
  }

  int size(){
    std::unique_lock<std::mutex> lock(m_Mutex);
    return m_Queue.size();
  }

  void push(T& value){
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Queue.emplace(value);
  }
  void push(T&& value){
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_Queue.emplace(std::move(value));
  }

  bool pop(T& value){
    std::unique_lock<std::mutex> lock(m_Mutex);
    if(m_Queue.empty()){
      return false;
    }else{
      value = std::move(m_Queue.front());
      m_Queue.pop();
      return true;
    }
  }

 private:
  std::queue<T> m_Queue;
  std::mutex m_Mutex;
};

/*
 * 实现的是单任务队列，需要考虑的问题
 * 1. 提交的任务：普通的函数，匿名函数(Lamda)， 仿函数（重载了()运算符的类或者结构体），类的成员函数, std::function/std::package_task等
 * 2. 这些任务的返回值不同，参数列表不同
 */


class SimpleThreadPool{
 public:
  // 不希望这个线程池被移动或者复制
  // 将拷贝构造函数，拷贝赋值运算符，移动构造函数，移动赋值运算符都删除，因为不希望被复制或者转移所有权
  SimpleThreadPool(const SimpleThreadPool&) = delete;
  SimpleThreadPool& operator=(const SimpleThreadPool&) = delete;
  SimpleThreadPool(const SimpleThreadPool&&) = delete;
  SimpleThreadPool& operator=(const SimpleThreadPool&&) = delete;
  // 无参构造函数
  SimpleThreadPool():m_Threads(6), m_RunningStatus(true){
    initialize();
  }
  // 有参构造函数
  SimpleThreadPool(int threadNum):m_Threads(threadNum), m_RunningStatus(true){
    initialize();
  }

  template<typename Func, typename... Args>
  /*
   * submitTask的作用：将外部传来的参数列表和函数进行打包，并且将任务的最终返回值通过future对象进行返回
   * 打包：
   *  1. 抹除参数列表，打包成了没有参数列表的函数 std::function<returnType()>
   *  2. 将返回值提前进行处理，packaged_task可以在并发环境中传递和执行，返回值的结果是future类型，将返回值get_futrue获取future对象
   */
  auto submitTask(Func&& func, Args... args){
    // 将函数包装成没有任何返回值
    // 对返回值进行包装，首先要知道返回值的类型
    // 第一步： 将任务打包成不带任何参数的任务
    using returnType = typename std::invoke_result<Func, Args...>::type;
    std::function<returnType()> taskWrapper1 = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);

    // 第二步：使用packaged_task进行打包.make_shared进一步将返回值打包做的一个处理
    auto taskWrapper2 = std::make_shared<std::packaged_task<returnType()>>(taskWrapper1);

    // 抹除返回值
    TaskType wrapperFunction = [taskWrapper2](){
      (*taskWrapper2)();
    };

    m_TaskQueue.push(wrapperFunction);
    m_CV.notify_one();
    return taskWrapper2->get_future();

  }

  ~SimpleThreadPool(){
    m_RunningStatus = false;

    m_CV.notify_all();
    for(auto& thread : m_Threads){
      if(thread.joinable()){
        thread.join();
      }
    }

  }
 private:
  // 将这个作为最终包装的任务类型
  using TaskType = std::function<void()>;
  SafeQueue<TaskType> m_TaskQueue;
  std::vector<std::thread> m_Threads;
  // 条件变量用于提交任务之后随机的唤醒一个线程
  std::condition_variable m_CV;
  std::mutex m_Mutex;
  // 控制线程池运行状态的一个变量
  std::atomic<bool> m_RunningStatus;


  void initialize(){
    for (int i = 0; i < m_Threads.size(); ++i) {
      auto worker = [this, i](){
        while(m_RunningStatus){
          TaskType task;
          bool isSucess = false;
          {
            std::unique_lock<std::mutex> locker(m_Mutex);
            if(this->m_TaskQueue.empty()){
              this->m_CV.wait(locker);
            }
            isSucess = this->m_TaskQueue.pop(task);
          }
          if(isSucess){
            std::cout <<"Start running task in worker ID = " << i << std::endl;
            task();
            std::cout << "end running task in worker id =" << i << std::endl;
          }
        }
      };
      m_Threads[i] = std::thread(worker);
    }
  }
};



