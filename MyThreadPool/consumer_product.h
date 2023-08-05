//
// Created by guessever on 23-8-5.
//
/*
 * 生产者将商品不断的输入队列，消费者从队列中拿走商品，可以有多个生产者多个消费者
 */

#pragma once
#include <iostream>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <thread>


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
 * 1. 队列已满，生产者停止生产，生产者wait条件变量
 * 2. 队列为空，消费者停止消费，消费者wait条件变量
 * 3. 队列不满但非空，生产者正常生产商品，消费者正常取商品
 */

// 不知道是存的是什么类型的数据，所以使用模板
template<typename T>
class ProducerConsumer{
 public:
  ProducerConsumer():m_QueueMaxSize(20), m_ProducerThreads(2), m_ConsumerThreads(2), m_RunningStatus(true){
    initialize();
  }
  ProducerConsumer(int queueMaxSize, int producerNum, int consumerNum):m_QueueMaxSize(queueMaxSize), m_ProducerThreads(producerNum), m_ConsumerThreads(consumerNum),m_RunningStatus(true){
    initialize();
  }
  ~ProducerConsumer(){
    m_RunningStatus = false;

    m_QueueNotFullCV.notify_all();
    m_QueueNotEmptyCV.notify_all();

    for(auto& thread : m_ProducerThreads){
      if(thread.joinable())
          thread.join();
    }
    for(auto& thread : m_ConsumerThreads){
      if(thread.joinable())
        thread.join();
    }
  }
 private:
  // 仓库 也就是队列
  SafeQueue<T> m_Queue;
 // 队列的最大大小
  int m_QueueMaxSize;

 // 两个条件变量
 std::condition_variable m_QueueNotFullCV;
 std::condition_variable m_QueueNotEmptyCV;

 std::vector<std::thread> m_ProducerThreads;
 std::vector<std::thread> m_ConsumerThreads;



 std::mutex m_Mutex;
 // 运行情况
 std::atomic<bool> m_RunningStatus;

 void initialize(){
   for(size_t i= 0 ; i < m_ProducerThreads.size(); i++){
     m_ProducerThreads[i] = std::thread(&ProducerConsumer::producer, this);
   }
   for(size_t i = 0; i < m_ConsumerThreads.size(); i++){
     m_ConsumerThreads[i] = std::thread(&ProducerConsumer::consumer, this);
   }
 }

 bool isFull(){
  if(m_Queue.size() >= m_QueueMaxSize){
    return true;
  }
  return false;
 }

 void producer(){
   while(m_RunningStatus){
     std::unique_lock<std::mutex> locker(m_Mutex);

     if(isFull()) {// 这里如果是while会怎么样？
       std::cout << "Queue is full, waiting for m_QueueNotFullCV; " << std::endl;
       m_QueueNotFullCV.wait(locker);
     }
     if(!isFull()){
       T value = 3;
       m_Queue.push(value);
       m_QueueNotEmptyCV.notify_one();
     }
   }
 }

 void consumer(){
   while(m_RunningStatus){
     std::unique_lock<std::mutex> locker(m_Mutex);
     if(m_Queue.empty()){
       std::cout << "Queue is empty, waiting for m_QueueNotEmptyCV" << std::endl;
       m_QueueNotEmptyCV.wait(locker);
     }
     if(!m_Queue.empty()){
       T value;
       bool result = m_Queue.pop(value);
       value++;
       std::cout << "result : " << value << std::endl;
       m_QueueNotFullCV.notify_one();
     }
   }
 }

};