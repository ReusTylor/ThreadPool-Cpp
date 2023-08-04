#include <iostream>
#include <future>
#include <thread>

// 一个简单的异步函数，计算两个整数的和
int add(int a, int b) {
  std::this_thread::sleep_for(std::chrono::seconds(2)); // 模拟耗时操作
  return a + b;
}

int main() {
  int x = 10, y = 20;

  // 使用 std::async 创建异步任务并获取 std::future 对象
  std::future<int> future_result = std::async(std::launch::async, add, x, y);

  // 执行其他操作
  std::cout << "其他操作正在执行..." << std::endl;

  // 等待异步任务完成，并获取结果
  int result = future_result.get();

  std::cout << "异步任务计算结果为: " << result << std::endl;

  return 0;
}