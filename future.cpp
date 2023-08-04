//
// Created by guessever on 23-8-4.
//
/*
 * future
 * 提供了异步访问操作结果的途径。
 * 在 C++11 的 std::future 被引入之前，通常的做法是： 创建一个线程 A，在线程 A 里启动任务 B，当准备完毕后发送一个事件，
 * 并将结果保存在全局变量中。 而主函数线程 A 里正在做其他的事情，当需要结果的时候，调用一个线程等待函数来获得执行的结果。
 */

#include <iostream>
#include <future>
#include <thread>

int main(){
  // 将一个返回值为7的lamda函数封装到task中
  // std::packaged_task的模板参数为要封装的函数的类型
  std::packaged_task<int()> task([](){return 7;});
  // 获得task的future
  std::future<int> result = task.get_future();// 在一个线程中执行task
  std::thread(std::move(task)).detach();
  std::cout<<"waiting ... ";
  result.wait();// 设置屏障 阻塞到future完成
  // 输出执行结果
  std::cout << "done!" << std::endl << "future is " << result.get() << std::endl;
}