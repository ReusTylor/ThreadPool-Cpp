#include <iostream>
#include <vector>
#include <chrono>

#include "ThreadPool.h"

int main()
{
    // 创建包含四个线程的线程池
    ThreadPool pool(4);
    // 创建std::vector存储每个任务返回的结果
    // std::future 异步操作对象，获取任务的返回值
    std::vector< std::future<int> > results;

    // 向线程池中添加了8个任务，通过lamda函数
    for(int i = 0; i < 8; ++i) {
        results.emplace_back(
            // 每个任务中先输出hello，然后休眠1秒钟，
            // 最后输出world，并返回i的平方作为任务的返回值
            pool.enqueue([i] {
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i*i;
            })
        );
    }

    // 循环遍历result，通过调用get方法来获取每个任务的返回值，并将结果输出
    // 这里使用了&&，result会被推导为一个右值引用类型，避免了不必要的复制
    // 在具有拷贝和移动语义时，最好声明为右值引用
    for(auto && result: results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
    
    return 0;
}