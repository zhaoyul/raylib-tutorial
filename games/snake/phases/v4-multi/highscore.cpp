#include "highscore.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <sys/stat.h>

// ============================================================
// HighScoreEntry 实现
// ============================================================
HighScoreEntry::HighScoreEntry(const std::string& n, int s, int l)
    : name(n), score(s), length(l) {
    // 生成日期字符串
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
    date = buffer;
}

std::string HighScoreEntry::toJson() const {
    std::stringstream ss;
    ss << "{\"name\":\"" << name << "\","
       << "\"score\":" << score << ","
       << "\"length\":" << length << ","
       << "\"date\":\"" << date << "\"}";
    return ss.str();
}

HighScoreEntry HighScoreEntry::fromJson(const std::string& json) {
    HighScoreEntry entry;

    // 简单的 JSON 解析
    size_t namePos = json.find("\"name\":\"");
    if (namePos != std::string::npos) {
        namePos += 8;
        size_t nameEnd = json.find("\"", namePos);
        entry.name = json.substr(namePos, nameEnd - namePos);
    }

    size_t scorePos = json.find("\"score\":");
    if (scorePos != std::string::npos) {
        scorePos += 8;
        size_t scoreEnd = json.find(",", scorePos);
        entry.score = std::stoi(json.substr(scorePos, scoreEnd - scorePos));
    }

    size_t lenPos = json.find("\"length\":");
    if (lenPos != std::string::npos) {
        lenPos += 9;
        size_t lenEnd = json.find(",", lenPos);
        entry.length = std::stoi(json.substr(lenPos, lenEnd - lenPos));
    }

    size_t datePos = json.find("\"date\":\"");
    if (datePos != std::string::npos) {
        datePos += 8;
        size_t dateEnd = json.find("\"", datePos);
        entry.date = json.substr(datePos, dateEnd - datePos);
    }

    return entry;
}

// ============================================================
// HighScoreManager 实现
// ============================================================
HighScoreManager::HighScoreManager(const std::string& fname)
    : filename(fname) {
    load();
}

bool HighScoreManager::load() {
    std::string fullPath = getFullPath();
    std::ifstream file(fullPath);

    if (!file.is_open()) {
        // 文件不存在，使用空列表
        return true;
    }

    entries.clear();
    std::string line;
    std::string json;

    while (std::getline(file, line)) {
        json += line;
    }

    // 解析 JSON 数组
    size_t pos = 0;
    while ((pos = json.find("{", pos)) != std::string::npos) {
        size_t end = json.find("}", pos);
        if (end == std::string::npos) break;

        std::string entryJson = json.substr(pos, end - pos + 1);
        entries.push_back(HighScoreEntry::fromJson(entryJson));
        pos = end + 1;
    }

    // 按分数排序
    std::sort(entries.begin(), entries.end(),
        [](const HighScoreEntry& a, const HighScoreEntry& b) {
            return a.score > b.score;
        });

    return true;
}

bool HighScoreManager::save() const {
    ensureDirectory();
    std::string fullPath = getFullPath();
    std::ofstream file(fullPath);

    if (!file.is_open()) {
        return false;
    }

    file << "[";
    for (size_t i = 0; i < entries.size(); i++) {
        if (i > 0) file << ",";
        file << entries[i].toJson();
    }
    file << "]";

    return true;
}

bool HighScoreManager::addEntry(const HighScoreEntry& entry) {
    entries.push_back(entry);

    // 按分数排序
    std::sort(entries.begin(), entries.end(),
        [](const HighScoreEntry& a, const HighScoreEntry& b) {
            return a.score > b.score;
        });

    // 只保留前 MAX_ENTRIES 个
    if (entries.size() > MAX_ENTRIES) {
        entries.resize(MAX_ENTRIES);
    }

    return save();
}

bool HighScoreManager::isHighScore(int score) const {
    if (entries.size() < MAX_ENTRIES) {
        return true;
    }
    return score > entries.back().score;
}

int HighScoreManager::getRank(int score) const {
    for (size_t i = 0; i < entries.size(); i++) {
        if (score > entries[i].score) {
            return static_cast<int>(i) + 1;
        }
    }
    if (entries.size() < MAX_ENTRIES) {
        return static_cast<int>(entries.size()) + 1;
    }
    return 0;
}

int HighScoreManager::getHighestScore() const {
    if (entries.empty()) {
        return 0;
    }
    return entries[0].score;
}

void HighScoreManager::clear() {
    entries.clear();
    save();
}

void HighScoreManager::ensureDirectory() const {
    // 获取父目录
    size_t pos = filename.find_last_of("/\\");
    if (pos != std::string::npos) {
        std::string dir = filename.substr(0, pos);
        #ifdef _WIN32
        _mkdir(dir.c_str());
        #else
        ::mkdir(dir.c_str(), 0755);
        #endif
    }
}

std::string HighScoreManager::getFullPath() const {
    // 使用应用程序目录下的 data 文件夹
    #ifdef _WIN32
    return "data/" + filename;
    #else
    return "data/" + filename;
    #endif
}
