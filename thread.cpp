//
// Created by guessever on 23-8-4.
//
#include <iostream>
#include <thread>
int main(){

  std::thread t([](){
    std::cout<< "hello world" << std::endl;
  });


  std::cout << "thread id = " << t.get_id() << std::endl;
  t.join();
  return 0;
}


