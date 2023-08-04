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


/*std::mutex mtx;
std::condition_variable cv_;
void task(int a, int b, int& ret){
  int ret_a = a * a;
  int ret_b = b * 2;
  std::unique_lock<std::mutex> lock(mtx);
  ret = ret_a + ret_b;
  cv_.notify_one();
}

int main(){
  int ret = 0;
  std::thread t(task, 1, 2, std::ref(ret));

  std::unique_lock<std::mutex> lock(mtx);
  cv_.wait(lock);
  std::cout << "Return value is " << ret << std::endl;

  t.join();
}*/

/*
 * 为了更简单的实现上面的功能，使用future
 * 流程：
 *  首先在主线程中增加了promise和future，通过promise.get_future()将这两个类型联系在一起
 *  向子线程传入promise的引用与主线程进行连接
 *  子线程对promise进行一个set_value的操作，赋值
 *  主线程对future进行get操作拿到子线程set_value后的值，这就完成了值的传递
 */

void task(int a, std::future<int> &b, std::promise<int> &ret) {
  int ret_a = a * a;
  int ret_b = b.get() * 2;

  ret.set_value(ret_a + ret_b);

}

int main() {
  int ret = 0;
  // 从子线程中获取
  //不可复制但是可以使用move

  std::promise<int> p_ret;
/*  std::promise<int> p_ret2 = std::move(p_ret);*/
  std::future<int> f_ret = p_ret.get_future();

  //子线程接收主线程传递的值
  std::promise<int> p_in;
  std::future<int> f_in = p_in.get_future();

/*  std::shared_future<int> s_f = f_in.share();*/

  // 通过一系列的值知道了p_in的值，可以进行set_value操作
  p_in.set_value(2);

  std::thread t(task, 1, std::ref(f_in), std::ref(p_ret));


  //get操作只能进行一次
  std::cout << "Return value is " << f_ret.get() << std::endl;

  t.join();
}

