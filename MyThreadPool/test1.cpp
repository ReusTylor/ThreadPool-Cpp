#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>


void f1(int n){
  for(int i = 0; i < 5; ++i){
    std::cout << "thread : " << n << " executing" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void f2(int& n){
  for(int i = 0; i < 5; ++i){
    std::cout << "Thread 2 executing" << std::endl;
    ++n;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}


int main(){
  int n = 0;
  std::thread t1;
  std::thread t2(f1, n + 1);
  std::thread t3(f2, std::ref(n));

  std::thread t4(std::move(t3));

  t2.join();
  t4.join();

  std::cout << "Final value of n is " << n << std::endl;

}