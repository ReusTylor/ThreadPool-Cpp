//
// Created by guessever on 23-8-5.
//
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <deque>
#include <condition_variable>

#include <future>
#include <functional>


/*void task(int a, int b, std::promise<int>& ret){
  int ret_a = a * a;
  int ret_b = b * 2;
  ret.set_value(ret_a + ret_b);
}

int main(){
  std::promise<int> p_ret;
  std::future<int> f_ret = p_ret.get_future();
  std::thread t(task, 1, 2, std::ref(p_ret));
  std::cout << "value is " << f_ret.get() << std::endl;
  t.join();
}*/

/*
 * 为了简化这种写法。给出了async
 */

int task(int a, int b){
  int ret_a = a * a;
  int ret_b = b * 2;
  return ret_b + ret_a;
}

/*
int main(){
*/
/*  std::promise<int> p_ret;
  std::future<int> f_ret = p_ret.get_future();
  std::thread t(task, 1, 2, std::ref(p_ret));*//*



  // 返回值是一个future类型
  // 这里是否需要创建线程是根据需要进行的，可以直接指定开启新的线程，加入参数
  std::future<int> fu = std::async(task, 1, 2);
  // 可以直接指定开启新的线程，加入参数std::launch::async
  std::future<int> fu1 = std::async(std::launch::async, task, 1, 2);
  // 加入参数,延迟操作，在需要时才执行这个任务，延迟调用，在同一个线程中执行
  std::future<int> fu2 = std::async(std::launch::deferred, task, 1, 2);
  std::cout << "value is " << fu.get() << std::endl;


*/
/*  t.join();*//*

}*/


int main(){
  // 完成对这个任务的包装，可以在其他地方进行调用
  std::packaged_task<int(int, int)> t1(task);
  // 通过get_future获取结果
  t1(1, 2);
  int result1 = t1.get_future().get();
  std::cout << "value is " << result1 << std::endl;

  // 可以使用bind函数将任务和参数绑定在一起
  // 返回的是一个std::function的类型
  std::packaged_task<int()> t2(std::bind(task, 1, 2));
  t2();
  int result2 = t2.get_future().get();
  std::cout << "value is " << result2 << std::endl;
}