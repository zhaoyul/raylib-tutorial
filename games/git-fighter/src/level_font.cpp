#include "level_manager.h"
#include <cstring>
#include <algorithm>

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
    
    // Generate codepoints - ASCII + CJK
    int codepoints[20000];
    int idx = 0;
    for (int cp = 0x0020; cp <= 0x007E; cp++) codepoints[idx++] = cp;
    for (int cp = 0x4E00; cp <= 0x8FFF; cp++) codepoints[idx++] = cp;
    
    // Load font at larger base size for better quality
    // Use 96px base size for sharper rendering at common sizes
    chineseFont = LoadFontEx(fontPath, 96, codepoints, idx);
    
    if (chineseFont.texture.id != 0 && chineseFont.glyphCount > 1000) {
        hasChineseFont = true;
        
        // Use trilinear filtering for best quality at all sizes
        SetTextureFilter(chineseFont.texture, TEXTURE_FILTER_TRILINEAR);
        
        // Enable font smoothing
        SetTextureWrap(chineseFont.texture, TEXTURE_WRAP_CLAMP);
        
        TraceLog(LOG_INFO, "Level font loaded: %d glyphs at 96px base size with trilinear filtering", chineseFont.glyphCount);
    }
}

void LevelFont::Unload() {
    if (hasChineseFont) {
        UnloadFont(chineseFont);
        hasChineseFont = false;
    }
}

// Check if text contains Chinese characters
static bool ContainsChinese(const char* text) {
    while (*text) {
        unsigned char c = (unsigned char)*text;
        // Check for multi-byte UTF-8 sequence (Chinese characters are typically 3 bytes)
        if (c >= 0x80) {
            return true;
        }
        text++;
    }
    return false;
}

void LevelFont::DrawChinese(const char* text, int x, int y, int fontSize, Color color) const {
    // If text contains no Chinese characters, use default raylib font for better English rendering
    if (!hasChineseFont || !ContainsChinese(text)) {
        DrawText(text, x, y, fontSize, color);
        return;
    }
    
    // Text contains Chinese - use the CJK font
    float spacing = 1.0f;
    Vector2 pos = {(float)x, (float)y};
    
    // Font size should not exceed base size (96) to avoid blurriness
    float drawSize = (float)fontSize;
    if (drawSize > 96.0f) drawSize = 96.0f;
    
    DrawTextEx(chineseFont, text, pos, drawSize, spacing, color);
}

// Measure Chinese text width
int LevelFont::MeasureChineseWidth(const char* text, int fontSize) const {
    if (hasChineseFont) {
        float spacing = 1.0f;
        float drawSize = (float)fontSize;
        if (drawSize > 96.0f) drawSize = 96.0f;
        Vector2 size = MeasureTextEx(chineseFont, text, drawSize, spacing);
        return (int)size.x;
    }
    return MeasureText(text, fontSize);
}
