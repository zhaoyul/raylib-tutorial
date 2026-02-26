#include "level_manager.h"
#include "chinese_font_loader.h"

void LevelFont::Load() {
    hasChineseFont = GitFighter::FontSupport::LoadChineseFont(chineseFont, 96);

    if (hasChineseFont) {
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

void LevelFont::DrawChinese(const char* text, int x, int y, int fontSize, Color color) const {
    // If text contains no Chinese characters, use default raylib font for better English rendering
    if (!hasChineseFont || !GitFighter::FontSupport::ContainsChinese(text)) {
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
