//
// Created by guessever on 23-8-5.
//

#include <iostream>
#include <thread>

void func(int a){
/*  while(true){*/
    std::cout << "hello world" << std::endl;
    //休眠50ms 每个线程
    std::this_thread::sleep_for(std::chrono::microseconds(50));
    std::this_thread::get_id();  // 获取当前线程的ID
/*  }*/
}

int main(){
  int a = 0;
  // 创建了一个名为thread1的线程，并开始执行func函数
  std::thread thread1(func, a);
  std::cout << thread1.get_id() << std::endl;
  std::cout << thread1.hardware_concurrency() <<std::endl;// 可能会报实例调用的提示

  // 因为这个方法是static，所以通过类型来调用更好
  std::cout << std::thread::hardware_concurrency() << std::endl;
  thread1.join(); // 会远远不断的打印hello world

  // thread1.detach(); // 将主线程和子线程分离开。没有输出了，后面没有任何东西了，程序已经结束了，所以线程都被回收。（尽量少用）

// 如果没有下面注释中的代码呢，会出错，因为主线程在创建完子线程后没有任何代码，所以就退出了，而子线程还没有执行完毕，所以出现了错误
// 为了解决这个错误，可以使用.join函数，等待子线程结束，或者.detach() 将主线程和子线程分离开
/*  while(true){
    std::cout << "cat!" << std::endl;
  }
  while(true)*/
}