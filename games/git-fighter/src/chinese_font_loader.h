#pragma once

#include "raylib.h"

namespace GitFighter {
namespace FontSupport {

// 尝试加载系统/项目内置中文字体。返回 true 表示加载成功。
bool LoadChineseFont(Font& font, int baseFontSize = 96);

// 按中文字符存在性判断，使用默认字体渲染英文，中文字体渲染中文。
bool ContainsChinese(const char* text);

} // namespace FontSupport
} // namespace GitFighter
