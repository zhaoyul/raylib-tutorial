#include <iostream>
#include <string>

// 1. 简单的函数示例
int add(int a, int b) {
    return a + b;
}

double multiply(double a, double b) {
    return a * b;
}

// 2. 类示例
class Player {
private:
    std::string name;
    int health;
    int score;

public:
    // 构造函数
    Player(std::string playerName) : name(playerName), health(100), score(0) {}

    // 获取玩家信息
    void displayInfo() {
        std::cout << "玩家: " << name << std::endl;
        std::cout << "生命值: " << health << std::endl;
        std::cout << "分数: " << score << std::endl;
    }

    // 受到伤害
    void takeDamage(int damage) {
        health -= damage;
        if (health < 0) health = 0;
        std::cout << name << " 受到 " << damage << " 点伤害！" << std::endl;
    }

    // 增加分数
    void addScore(int points) {
        score += points;
        std::cout << name << " 获得 " << points << " 分！" << std::endl;
    }

    // 检查是否存活
    bool isAlive() {
        return health > 0;
    }
};

int main() {
    std::cout << "=== C++ 基础示例 ===" << std::endl << std::endl;

    // 1. 基本输出
    std::cout << "1. Hello, Raylib Tutorial!" << std::endl << std::endl;

    // 2. 变量和数据类型
    std::cout << "2. 变量示例:" << std::endl;
    int playerScore = 1000;
    float playerSpeed = 5.5f;
    bool gameOver = false;

    std::cout << "   分数: " << playerScore << std::endl;
    std::cout << "   速度: " << playerSpeed << std::endl;
    std::cout << "   游戏结束: " << (gameOver ? "是" : "否") << std::endl << std::endl;

    // 3. 函数示例
    std::cout << "3. 函数示例:" << std::endl;
    int sum = add(10, 20);
    double product = multiply(3.5, 2.0);
    std::cout << "   10 + 20 = " << sum << std::endl;
    std::cout << "   3.5 * 2.0 = " << product << std::endl << std::endl;

    // 4. 类和对象示例
    std::cout << "4. 类和对象示例:" << std::endl;
    Player player1("小明");
    player1.displayInfo();
    std::cout << std::endl;

    player1.addScore(100);
    player1.takeDamage(30);
    std::cout << std::endl;

    player1.displayInfo();
    std::cout << "玩家存活状态: " << (player1.isAlive() ? "存活" : "死亡") << std::endl;

    return 0;
}
