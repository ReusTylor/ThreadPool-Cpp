//
// Created by guessever on 23-8-5.
//

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

std::mutex mtx;

/*int globalVariable = 0;*/

/*
 * 可能会产生死锁
 * 1. 程序在临界区中直接推出，没有解锁操作 使用lock_guard<mutex> lock(mtx1); RAII 资源获取即初始化  lock_guard并不灵活，作用于整个函数
 *    unique_lock<mutex> lock 提供了更多的方法
 * 2. 多个锁之间上锁顺序的不同引发死锁 std::lock(mtx1, mtx2)对多个互斥量上锁
 */
/*
void task1(){
  for(int i = 0; i < 1000000; i++){
    mtx.lock();
    // 上锁和解锁之间称为临界区
    globalVariable++;
    globalVariable--;
    mtx.unlock();
  }
}


// 直接使用这个函数输出呢，讲道理应该globalVariable应该是0,但不是，就是因为不同线程之间没有隔离。，使用mutex来解决
// globalVariable是一个公共资源，因此这段代码是不安全的，使用互斥量
int main(){
  std::thread t1(task1);
  std::thread t2(task1);

  t1.join();
  t2.join();

  std::cout << "value = " << globalVariable << std::endl;
}

*/

/*
 * 另一个解决资源竞争的方法
 */
// 对共享资源进行改造

std::atomic<int> globalVariable = 0;

void task1(){
  for(int i = 0; i < 1000000; i++){
    globalVariable++;
    globalVariable--;
  }
}



int main(){
  std::thread t1(task1);
  std::thread t2(task1);

  t1.join();
  t2.join();

  std::cout << "value = " << globalVariable << std::endl;
}