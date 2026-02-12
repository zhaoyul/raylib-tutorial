#pragma once
#include <string>
#include <vector>
#include <ctime>

// ============================================================
// 高分记录结构
// ============================================================
struct HighScoreEntry {
    std::string name;       // 玩家名字
    int score;              // 分数
    int length;             // 蛇的长度
    std::string date;       // 日期字符串

    HighScoreEntry() : score(0), length(0) {}
    HighScoreEntry(const std::string& n, int s, int l);

    // 序列化到 JSON
    std::string toJson() const;
    // 从 JSON 反序列化
    static HighScoreEntry fromJson(const std::string& json);
};

// ============================================================
// 高分榜管理器
// ============================================================
class HighScoreManager {
private:
    static constexpr int MAX_ENTRIES = 10;
    std::vector<HighScoreEntry> entries;
    std::string filename;

public:
    HighScoreManager(const std::string& filename = "highscores.json");

    // 加载和保存
    bool load();
    bool save() const;

    // 添加新记录
    bool addEntry(const HighScoreEntry& entry);

    // 检查分数是否能上榜
    bool isHighScore(int score) const;

    // 获取排名（1-10，0表示未上榜）
    int getRank(int score) const;

    // 获取所有记录
    const std::vector<HighScoreEntry>& getEntries() const { return entries; }

    // 获取最高分
    int getHighestScore() const;

    // 清除所有记录
    void clear();

    // 获取记录数量
    int getCount() const { return static_cast<int>(entries.size()); }

private:
    // 确保目录存在
    void ensureDirectory() const;
    std::string getFullPath() const;
};
