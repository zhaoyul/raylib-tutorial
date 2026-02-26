#include "chinese_font_loader.h"

#include <array>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <filesystem>
#include <unordered_set>
#include <vector>
#include <string>

namespace GitFighter {
namespace FontSupport {

namespace {
constexpr int kMinGlyphCount = 1000;
constexpr int kMaxCodepoints = 26000;

void AddCandidate(std::vector<std::string>& candidates, const std::string& path, std::unordered_set<std::string>& seen);

std::string ToLowerCopy(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return out;
}

bool HasCandidateFontKeyword(const std::string& filenameLower) {
    static const std::array<const char*, 27> kKeywords = {
        "noto", "pingfang", "simsun", "simhei", "msyh", "zh", "zh-cn", "cjk", "hei",
        "song", "wqy", "microhei", "zenhei", "sourcehan", "source-han", "droid",
        "noto sans", "arial unicode", "microsoft yahei", "yahei", "fangsong",
        "kaiti", "hanyi", "alimt", "siyuan", "zhankai", "fandol"
    };

    for (const auto* keyword : kKeywords) {
        if (filenameLower.find(keyword) != std::string::npos) {
            return true;
        }
    }

    return false;
}

void AddSystemFontCandidatesFromDirectory(
    const std::filesystem::path& dir,
    std::vector<std::string>& candidates,
    std::unordered_set<std::string>& seen,
    bool preferKeywordMatch = true,
    int maxDepth = 6,
    int maxFonts = 800
) {
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
        return;
    }

    int collected = 0;
    try {
        for (std::filesystem::recursive_directory_iterator it(dir); it != std::filesystem::recursive_directory_iterator(); ++it) {
            const auto& entry = *it;
            if (!entry.is_regular_file()) {
                continue;
            }
            if (it.depth() > maxDepth) {
                continue;
            }

            const auto ext = ToLowerCopy(entry.path().extension().string());
            if (ext != ".ttf" && ext != ".otf" && ext != ".ttc") {
                continue;
            }

            const auto lowerName = ToLowerCopy(entry.path().filename().string());
            const bool keywordMatch = HasCandidateFontKeyword(lowerName);

            if (preferKeywordMatch && !keywordMatch) {
                continue;
            }

            AddCandidate(candidates, entry.path().string(), seen);
            ++collected;
            if (collected >= maxFonts) {
                break;
            }
        }
    } catch (const std::exception&) {
        // Ignore inaccessible directories/files caused by permissions.
    }
}

void AddSystemFontFallbackCandidates(
    const std::filesystem::path& dir,
    std::vector<std::string>& candidates,
    std::unordered_set<std::string>& seen,
    int maxDepth = 6,
    int maxFonts = 1200
) {
    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
        return;
    }

    int collected = 0;
    try {
        for (std::filesystem::recursive_directory_iterator it(dir); it != std::filesystem::recursive_directory_iterator(); ++it) {
            const auto& entry = *it;
            if (!entry.is_regular_file()) {
                continue;
            }
            if (it.depth() > maxDepth) {
                continue;
            }

            const auto ext = ToLowerCopy(entry.path().extension().string());
            if (ext != ".ttf" && ext != ".otf" && ext != ".ttc") {
                continue;
            }

            AddCandidate(candidates, entry.path().string(), seen);
            ++collected;
            if (collected >= maxFonts) {
                break;
            }
        }
    } catch (const std::exception&) {
        // Ignore inaccessible directories/files caused by permissions.
    }
}

void AddCandidate(std::vector<std::string>& candidates, const std::string& path, std::unordered_set<std::string>& seen) {
    if (path.empty()) return;
    auto normalized = std::filesystem::path(path).lexically_normal().string();
    if (!seen.insert(normalized).second) return;
    candidates.push_back(std::move(normalized));
}

bool LoadFontCandidate(Font& font, const std::string& fontPath, int baseFontSize) {
    if (fontPath.empty() || !std::filesystem::exists(fontPath)) {
        return false;
    }

    static int codepoints[kMaxCodepoints];
    int idx = 0;

    for (int cp = 0x0020; cp <= 0x007E && idx < kMaxCodepoints; cp++) {
        codepoints[idx++] = cp;
    }

    for (int cp = 0x4E00; cp <= 0x9FFF && idx < kMaxCodepoints; cp++) {
        codepoints[idx++] = cp;
    }

    font = LoadFontEx(fontPath.c_str(), baseFontSize, codepoints, idx);
    if (font.texture.id != 0 && font.glyphCount >= kMinGlyphCount) {
        return true;
    }

    if (font.texture.id != 0) {
        UnloadFont(font);
        font = {};
    }

    return false;
}

} // namespace

bool ContainsChinese(const char* text) {
    if (!text) return false;
    while (*text) {
        unsigned char c = static_cast<unsigned char>(*text);
        if (c >= 0x80) {
            return true;
        }
        ++text;
    }
    return false;
}

bool LoadChineseFont(Font& font, int baseFontSize) {
    font = {};

    std::vector<std::string> candidates;
    std::unordered_set<std::string> seen;

#if defined(_WIN32)
        const char* systemFonts[] = {
            "C:/Windows/Fonts/msyh.ttc",
            "C:/Windows/Fonts/msyhbd.ttc",
            "C:/Windows/Fonts/msyhl.ttc",
            "C:/Windows/Fonts/simsun.ttc",
            "C:/Windows/Fonts/simhei.ttf",
            "C:/Windows/Fonts/SimSun.ttf",
            "C:/Windows/Fonts/SimHei.ttf",
            "C:/Windows/Fonts/simkai.ttf",
            "C:/Windows/Fonts/NotoSansCJK-Regular.ttc"
        };
        for (const auto* path : systemFonts) {
            AddCandidate(candidates, path, seen);
        }

        // 常见系统字体目录扫描（按中文字体关键字筛选）
        const char* windir = std::getenv("WINDIR");
        if (windir) {
            AddSystemFontCandidatesFromDirectory(std::filesystem::path(windir) / "Fonts", candidates, seen);
        }

        const char* localAppData = std::getenv("LOCALAPPDATA");
        if (localAppData) {
            AddSystemFontCandidatesFromDirectory(
                std::filesystem::path(localAppData) / "Microsoft/Windows/Fonts",
                candidates,
                seen
            );
        }
#elif defined(__APPLE__)
        const char* systemFonts[] = {
            "/System/Library/Fonts/PingFang.ttc",
            "/System/Library/Fonts/PingFangSC.ttc",
            "/System/Library/Fonts/PingFang.ttc",
            "/System/Library/Fonts/Supplemental/PingFang.ttc",
            "/System/Library/Fonts/Supplemental/PingFangSC.ttc",
            "/System/Library/Fonts/Supplemental/Arial Unicode MS.ttf",
            "/Library/Fonts/Arial Unicode MS.ttf"
        };
        for (const auto* path : systemFonts) {
            AddCandidate(candidates, path, seen);
        }

        const char* home = std::getenv("HOME");
        if (home) {
            AddSystemFontCandidatesFromDirectory(std::filesystem::path(home) / "Library/Fonts", candidates, seen);
        }
        AddSystemFontCandidatesFromDirectory("/System/Library/Fonts/Supplemental", candidates, seen);
        AddSystemFontCandidatesFromDirectory("/Library/Fonts", candidates, seen);
#else
        const char* systemFonts[] = {
            "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
            "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
            "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.otf",
            "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.otf",
            "/usr/share/fonts/truetype/arphic/uming.ttc",
            "/usr/share/fonts/opentype/arphic/uming.ttc",
            "/usr/share/fonts/truetype/wqy/wqy-microhei.ttc",
            "/usr/share/fonts/wqy-microhei.ttc",
            "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
            "/usr/share/fonts/truetype/droid/DroidSansFallback.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf"
        };
        for (const auto* path : systemFonts) {
            AddCandidate(candidates, path, seen);
        }

        const char* home = std::getenv("HOME");
        if (home) {
            AddSystemFontCandidatesFromDirectory(std::filesystem::path(home) / ".fonts", candidates, seen);
            AddSystemFontCandidatesFromDirectory(std::filesystem::path(home) / ".local/share/fonts", candidates, seen);
        }

        AddSystemFontCandidatesFromDirectory("/usr/share/fonts", candidates, seen);
        AddSystemFontCandidatesFromDirectory("/usr/local/share/fonts", candidates, seen);
        AddSystemFontCandidatesFromDirectory("/usr/share/local/fonts", candidates, seen);
#endif

    // 兜底：如果上面都没能匹配到可用字体，再宽泛扫描系统字体目录。
    const char* home = std::getenv("HOME");
    if (home) {
        AddSystemFontFallbackCandidates(
            std::filesystem::path(home) / "Library/Fonts",
            candidates,
            seen
        );
        AddSystemFontFallbackCandidates(
            std::filesystem::path(home) / ".fonts",
            candidates,
            seen
        );
        AddSystemFontFallbackCandidates(
            std::filesystem::path(home) / ".local/share/fonts",
            candidates,
            seen
        );
    }

    AddSystemFontFallbackCandidates("/System/Library/Fonts", candidates, seen);
    AddSystemFontFallbackCandidates("/Library/Fonts", candidates, seen);
    AddSystemFontFallbackCandidates("/usr/share/fonts", candidates, seen);
    AddSystemFontFallbackCandidates("/usr/local/share/fonts", candidates, seen);

    for (const auto& path : candidates) {
        if (LoadFontCandidate(font, path, baseFontSize)) {
            TraceLog(LOG_INFO, "Chinese font loaded from: %s", path.c_str());
            return true;
        }
    }

    TraceLog(LOG_WARNING, "No valid Chinese font found in built-in/system candidates");
    return false;
}

} // namespace FontSupport
} // namespace GitFighter
