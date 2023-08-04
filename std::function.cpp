//
// Created by guessever on 23-8-4.
//


/*
 * 需求：
 *  分两次打印出一个vector集合中的所有
 *   1. 模5 = 0
 *   2. 大于20
 *  的数字
 */

#include <iostream>
#include <vector>
#include <functional>
using namespace std;

void printNumber1(vector<int>& nums){
  for(const int& i : nums){
    if(i % 5 == 0){
      cout << i <<" ";
    }
  }

}

void printNumber2(vector<int>& nums){
  for(const int& i : nums){
    if(i > 20){
      cout << i <<" ";
    }
  }
}

/*
 * 使用std::function
 */

void printNumber(vector<int>& nums, function<bool(int)> filter){
  for(const int& i : nums){
    if(filter(i)){
      cout << i << " ";
    }
  }
}

int main(){
  vector<int> nums{ 1, 2, 3, 4, 5, 10, 15, 20, 25, 35, 45, 50 };
  printNumber1(nums);
  cout << endl;
  printNumber2(nums);
  cout << endl;
  printNumber(nums, [](int i){return i % 5 == 0;});
  cout << endl;
  printNumber(nums, [](int i){return i > 20;});
  return 0;

}