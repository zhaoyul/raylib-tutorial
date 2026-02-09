#include <iostream>
#include "math_utils.h"

int main() {
    std::cout << "=== CMake 多文件项目示例 ===" << std::endl << std::endl;
    
    // 使用外部文件中的函数
    std::cout << "5 + 3 = " << add(5, 3) << std::endl;
    std::cout << "5 - 3 = " << subtract(5, 3) << std::endl;
    std::cout << "5 * 3 = " << multiply(5, 3) << std::endl;
    std::cout << "10 / 2 = " << divide(10, 2) << std::endl;
    
    std::cout << std::endl;
    std::cout << "这个程序演示了如何使用 CMake 构建多文件项目。" << std::endl;
    std::cout << "main.cpp 调用了 math_utils.cpp 中定义的函数。" << std::endl;
    
    return 0;
}
