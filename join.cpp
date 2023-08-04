//
// Created by guessever on 23-8-4.
//

#include <iostream>
#include <thread>

int caculateSum(int a, int b){
  int result = a * b;
  std::cout << "sum" << result << std::endl;
}

int main(){
  int num = 5;
  std::thread workerThread(caculateSum, 2, 3);
  // 如果没有这个，主线程有可能先于工作线程结束。
  workerThread.join();

  std::cout << "Main thread is done" << std::endl;
}
