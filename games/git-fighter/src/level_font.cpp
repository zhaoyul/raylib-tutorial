#include "level_manager.h"
#include <cstring>

void LevelFont::Load() {
    hasChineseFont = false;
    
    // Try multiple possible paths
    const char* paths[] = {
        "data/fonts/NotoSansCJKsc-Regular.otf",
        "../data/fonts/NotoSansCJKsc-Regular.otf",
        "../../data/fonts/NotoSansCJKsc-Regular.otf",
        "../../../data/fonts/NotoSansCJKsc-Regular.otf",
    };
    
    const char* fontPath = nullptr;
    for (const auto& p : paths) {
        if (FileExists(p)) {
            fontPath = p;
            break;
        }
    }
    
    if (!fontPath) {
        TraceLog(LOG_WARNING, "Chinese font not found in any standard path");
        return;
    }
    
    // Generate codepoints
    int codepoints[20000];
    int idx = 0;
    for (int cp = 0x0020; cp <= 0x007E; cp++) codepoints[idx++] = cp;
    for (int cp = 0x4E00; cp <= 0x8FFF; cp++) codepoints[idx++] = cp;
    
    chineseFont = LoadFontEx(fontPath, 48, codepoints, idx);
    
    if (chineseFont.texture.id != 0 && chineseFont.glyphCount > 1000) {
        hasChineseFont = true;
        TraceLog(LOG_INFO, "Level font loaded: %d glyphs", chineseFont.glyphCount);
    }
}

void LevelFont::Unload() {
    if (hasChineseFont) {
        UnloadFont(chineseFont);
        hasChineseFont = false;
    }
}

void LevelFont::DrawChinese(const char* text, int x, int y, int fontSize, Color color) const {
    if (hasChineseFont) {
        DrawTextEx(chineseFont, text, (Vector2){(float)x, (float)y}, (float)fontSize, 2.0f, color);
    } else {
        DrawText(text, x, y, fontSize, color);
    }
}
