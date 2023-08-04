//
// Created by guessever on 23-8-5.
//
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <deque>
#include <condition_variable>
/*
 * 单纯的while循环很占CPU，可以使用延时的方式来解决
 *
 */

std::condition_variable cv;
std::mutex mtx;
std::deque<int> q;

//producer
void task1(){
  int i = 0;
  // 源源不断的往双向队列中存数据
  while(true){
    std::unique_lock<std::mutex> lock(mtx);
    q.push_back(i);
    // 唤醒等待或者休眠中的某一个线程
    // cv.notify_all();
    cv.notify_one();
/*    std::this_thread::sleep_for(std::chrono::milliseconds(10));*/
    if(i < 99999999){
      i++;
    }else{
      i = 0;
    }
  }
}


// consumer 1
void task2() {
  int data = 0;
  // 不断消耗数据并打印
  while (true) {
    std::unique_lock<std::mutex> lock(mtx);

    while (q.empty()) {
      // 使当前的线程陷入等待或者休眠状态，传入lock作为参数，其实是会帮我们释放掉这个锁
      cv.wait(lock);
    }

    data = q.front();
    q.pop_front();
    std::cout << "Get value from que :" << data << std::endl;

/*    std::this_thread::sleep_for(std::chrono::milliseconds(10));*/
  }
}

// consumer 2
/*
 * 当有两个消费者线程时， 会有一种虚假唤醒的情况。：当某个条件没有满足，但是线程也被唤醒了，可以将if换成while即可
 */
void task3() {
  int data = 0;
  // 不断消耗数据并打印
  while (true) {
    std::unique_lock<std::mutex> lock(mtx);

    while (q.empty()) {
      // 使当前的线程陷入等待或者休眠状态，传入lock作为参数，其实是会帮我们释放掉这个锁
      cv.wait(lock);
    }

    data = q.front();
    q.pop_front();
    std::cout << "Get value from que :" << data << std::endl;

/*    std::this_thread::sleep_for(std::chrono::milliseconds(10));*/
  }
}
int main(){
  std::thread t1(task1);
  std::thread t2(task2);
  std::thread t3(task3);
  t1.join();
  t2.join();
  t3.join();

}


