//
// Created by guessever on 23-8-4.
//

#include <iostream>
#include <mutex>
#include <thread>

int v = 1;
void critical_section(int change_v){
  static std::mutex mtx;
  std::unique_lock<std::mutex> lock(mtx);

  v = change_v;
  std::cout << v << std::endl;
  // 将锁进行释放
  lock.unlock();

  // 在此期间，任何人都可以抢夺v的持有权

  // 开始另一组竞争操作，再次加锁
  lock.lock();
  v += 1;
  std::cout << v << std::endl;
}

int main(){
  std::thread t1(critical_section, 2), t2(critical_section, 3);
  t1.join();
  t2.join();
  return 0;
}