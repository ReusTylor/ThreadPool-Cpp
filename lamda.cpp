//
// Created by guessever on 23-8-4.
//


#include <iostream>
#include <memory>  // std::make_unique
#include <utility> // std::move
#include <functional>

int foo(int a, int b, int c) {
  std::cout << a + b + c << std::endl;
}
int main() {
  // 将参数1,2绑定到函数 foo 上，
  // 但使用 std::placeholders::_1 来对第一个参数进行占位
  auto bindFoo = std::bind(foo, std::placeholders::_1, 1,2);
  // 这时调用 bindFoo 时，只需要提供第一个参数即可
  bindFoo(1);
}