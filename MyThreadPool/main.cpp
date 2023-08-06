//
// Created by guessever on 23-8-5.
//
#include "SimpleThreadPool.h"

int very_time_consuming_task(int a, int b){
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return a + b;
}

int main(){
  SimpleThreadPool simple_thread_pool(12);
  int taskNum = 30;
  std::vector<std::future<int>> result(30);


  std::cout << "start to submit tasks..." << std::endl;
  for (size_t i = 0; i < taskNum; i++) {
    result[i] = simple_thread_pool.submitTask(very_time_consuming_task, i, i + 1);
  }
  std::cout << "end submit tasks..." << std::endl;


  std::cout << "main thread do something else" << std::endl;
  // do something else
  std::this_thread::sleep_for(std::chrono::seconds(10));

  std::cout << "main thread task finished" << std::endl;


  std::cout << "try get threadpool task result..." << std::endl;
  for (int i = 0; i < taskNum; ++i) {
    std::cout << " result[ " << i << "]" << result[i].get();
  }
  std::cout << "end of getting result " << std::endl;


}