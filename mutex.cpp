//
// Created by guessever on 23-8-4.
//

/*
 * 互斥量和临界区
 */

#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

int v = 1;
void critical_section(int change_v){
  // 保证同时只有一个线程可以访问临界区
  static std::mutex mtx;
  // RAII语法的模板类std::lock_guard, 对于临界区的互斥量创建只需要在作用域开始的部分
  // lock_guard不能显式调用lock和unlock，而std::unique_lock可以在声明后的任意位置调用，可以缩小锁的范围，提供更高的并发度。
  // 如果用到了条件变量：std::condition_variable::wait 则必须使用std::unique_lock作为参数
  std::lock_guard<std::mutex> lock(mtx);

  // 执行竞争操作
  v = change_v;
}

int main(){
  std::thread t1(critical_section, 3), t2(critical_section, 2);
  t1.join();
  t2.join();

  std::cout << v << std::endl;
  return 0;
}