#include "raylib.h"
#include <stdlib.h>  // For malloc/free

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// Item Types
enum ItemType {
    ITEM_WEAPON,
    ITEM_ARMOR,
    ITEM_POTION,
    ITEM_MISC
};

// Item structure
struct Item {
    const char* name;
    ItemType type;
    int value;
    Color color;
};

// UI Text management
struct UIText {
    Font chineseFont;
    bool hasChineseFont;
    const char* fontPath;

    // Menu
    const char* title;
    const char* goldLabel;
    const char* tabs[4];

    // Items tab
    const char* backpackTitle;
    const char* itemDetails;
    const char* itemList;
    const char* selectPrompt;
    const char* capacityLabel;
    const char* totalValue;
    const char* typeWeapon;
    const char* typeArmor;
    const char* typePotion;
    const char* typeMisc;
    const char* valueLabel;
    const char* btnUse;
    const char* btnDrop;
    const char* btnSell;
    const char* msgUseItem;
    const char* msgDropItem;
    const char* msgSold;

    // Equipment tab
    const char* equipTitle;
    const char* equipSlots[6];
    const char* statsTitle;
    const char* statAttack;
    const char* statDefense;
    const char* statHP;
    const char* statMP;
    const char* appearanceLabel;

    // Shop tab
    const char* shopTitle;
    const char* btnBuy;
    const char* shopInfo;
    const char* shopHint1;
    const char* shopHint2;
    const char* shopHint3;

    // Settings tab
    const char* lblPlayerName;
    const char* lblVolume;
    const char* lblDifficulty;
    const char* lblTheme;
    const char* chkSound;
    const char* btnSave;
    const char* btnReset;
    const char* msgSaved;
    const char* diffOptions;
    const char* themeOptions;
    const char* lblTestValue;
    const char* diffEasy;
    const char* diffMedium;
    const char* diffHard;
};

// Common Chinese characters for UI
// Includes: ASCII + Full CJK (0x4E00-0x9FFF) + Punctuation
int GetChineseCodepoints(int** codepoints) {
    // ASCII range
    int asciiCount = 0x007E - 0x0020 + 1;  // 95 chars

    // Full CJK Unified Ideographs (0x4E00-0x9FFF) - ~20k chars
    int cjkStart = 0x4E00;
    int cjkEnd = 0x9FFF;  // Full CJK range
    int cjkCount = cjkEnd - cjkStart + 1;

    // CJK Punctuation (0x3000-0x303F)
    int punctStart = 0x3000;
    int punctEnd = 0x303F;
    int punctCount = punctEnd - punctStart + 1;

    int total = asciiCount + cjkCount + punctCount;
    *codepoints = (int*)malloc(total * sizeof(int));

    int idx = 0;
    for (int cp = 0x0020; cp <= 0x007E; cp++) (*codepoints)[idx++] = cp;
    for (int cp = cjkStart; cp <= cjkEnd; cp++) (*codepoints)[idx++] = cp;
    for (int cp = punctStart; cp <= punctEnd; cp++) (*codepoints)[idx++] = cp;

    TraceLog(LOG_INFO, "Generated %d codepoints for Chinese font", idx);
    return idx;
}

// Try to load font with Chinese support
bool TryLoadFont(UIText& ui, const char* path) {
    if (!FileExists(path)) {
        return false;
    }

    TraceLog(LOG_INFO, "Loading font: %s", path);

    // Generate codepoints for Chinese characters
    int* codepoints = NULL;
    int codepointCount = GetChineseCodepoints(&codepoints);

    TraceLog(LOG_INFO, "Loading %d codepoints...", codepointCount);

    ui.chineseFont = LoadFontEx(path, 32, codepoints, codepointCount);
    free(codepoints);

    if (ui.chineseFont.texture.id != 0) {
        TraceLog(LOG_INFO, "Font loaded: %s (glyphs: %d)", path, ui.chineseFont.glyphCount);

        if (ui.chineseFont.glyphCount > 1000) {
            ui.hasChineseFont = true;
            ui.fontPath = path;
            GuiSetFont(ui.chineseFont);
            TraceLog(LOG_INFO, "Chinese font active: %s", path);
            return true;
        } else {
            TraceLog(LOG_WARNING, "Font has only %d glyphs", ui.chineseFont.glyphCount);
            UnloadFont(ui.chineseFont);
            ui.chineseFont = (Font){0};
        }
    }

    return false;
}

// Predefined items (English default)
Item items_en[] = {
    {"Iron Sword", ITEM_WEAPON, 100, GRAY},
    {"Steel Sword", ITEM_WEAPON, 250, LIGHTGRAY},
    {"Magic Sword", ITEM_WEAPON, 500, PURPLE},
    {"Leather Armor", ITEM_ARMOR, 80, BROWN},
    {"Iron Armor", ITEM_ARMOR, 200, DARKGRAY},
    {"Health Potion", ITEM_POTION, 50, RED},
    {"Mana Potion", ITEM_POTION, 50, BLUE},
    {"Gold Bag", ITEM_MISC, 1000, GOLD},
    {"Key", ITEM_MISC, 10, YELLOW},
    {"Gem", ITEM_MISC, 500, GREEN}
};

// Predefined items (Chinese)
Item items_cn[] = {
    {"铁剑", ITEM_WEAPON, 100, GRAY},
    {"钢剑", ITEM_WEAPON, 250, LIGHTGRAY},
    {"魔法剑", ITEM_WEAPON, 500, PURPLE},
    {"皮甲", ITEM_ARMOR, 80, BROWN},
    {"铁甲", ITEM_ARMOR, 200, DARKGRAY},
    {"生命药水", ITEM_POTION, 50, RED},
    {"魔法药水", ITEM_POTION, 50, BLUE},
    {"金币袋", ITEM_MISC, 1000, GOLD},
    {"钥匙", ITEM_MISC, 10, YELLOW},
    {"宝石", ITEM_MISC, 500, GREEN}
};

Item* items = items_en;

void LoadChineseFont(UIText& ui) {
    ui.hasChineseFont = false;
    ui.fontPath = NULL;

    // Try downloaded fonts first
    if (TryLoadFont(ui, "data/fonts/AlibabaPuHuiTi-Regular.otf")) goto set_chinese;
    if (TryLoadFont(ui, "data/fonts/NotoSansSC-Regular.otf")) goto set_chinese;
    if (TryLoadFont(ui, "data/fonts/NotoSansCJKsc-Regular.otf")) goto set_chinese;

    TraceLog(LOG_WARNING, "No Chinese font found in data/fonts/");
    TraceLog(LOG_WARNING, "Please download a Chinese font - see data/fonts/README.md");
    goto set_english;

set_chinese:
    items = items_cn;
    ui.title = "RPG 物品栏系统";
    ui.goldLabel = "金币: %d";
    ui.tabs[0] = "物品"; ui.tabs[1] = "装备"; ui.tabs[2] = "商店"; ui.tabs[3] = "设置";
    ui.backpackTitle = "背包";
    ui.itemDetails = "物品详情";
    ui.itemList = "铁剑 (武器);钢剑 (武器);魔法剑 (武器);皮甲 (护甲);铁甲 (护甲);生命药水 (药水);魔法药水 (药水);金币袋 (杂物);钥匙 (杂物);宝石 (杂物)";
    ui.selectPrompt = "请选择一个物品";
    ui.capacityLabel = "背包容量: %d/%d";
    ui.totalValue = "总价值: %d 金币";
    ui.typeWeapon = "武器"; ui.typeArmor = "护甲"; ui.typePotion = "药水"; ui.typeMisc = "杂物";
    ui.valueLabel = "价值: %d 金币";
    ui.btnUse = "使用"; ui.btnDrop = "丢弃"; ui.btnSell = "出售";
    ui.msgUseItem = "确定要使用 %s 吗？"; ui.msgDropItem = "确定要丢弃 %s 吗？"; ui.msgSold = "出售 %s 获得 %d 金币";
    ui.equipTitle = "装备栏";
    ui.equipSlots[0] = "头盔"; ui.equipSlots[1] = "左手"; ui.equipSlots[2] = "胸甲"; ui.equipSlots[3] = "右手"; ui.equipSlots[4] = "护腿"; ui.equipSlots[5] = "鞋子";
    ui.statsTitle = "属性";
    ui.statAttack = "攻击力: %d"; ui.statDefense = "防御力: %d"; ui.statHP = "生命值: %d/%d"; ui.statMP = "魔法值: %d/%d";
    ui.appearanceLabel = "外观颜色";
    ui.shopTitle = "商店"; ui.btnBuy = "购买"; ui.shopInfo = "商店说明";
    ui.shopHint1 = "- 点击购买获得物品"; ui.shopHint2 = "- 金币不足时无法购买"; ui.shopHint3 = "- 每天刷新商品";
    ui.lblPlayerName = "玩家名称:"; ui.lblVolume = "音量:"; ui.lblDifficulty = "游戏难度:"; ui.lblTheme = "界面主题:";
    ui.chkSound = "启用音效"; ui.btnSave = "保存设置"; ui.btnReset = "重置默认"; ui.msgSaved = "设置已保存！";
    ui.diffOptions = "简单;中等;困难"; ui.themeOptions = "默认;暗色;蓝色"; ui.lblTestValue = "测试数值:";
    ui.diffEasy = "简单"; ui.diffMedium = "中等"; ui.diffHard = "困难";
    return;

set_english:
    items = items_en;
    ui.title = "RPG INVENTORY SYSTEM";
    ui.goldLabel = "Gold: %d";
    ui.tabs[0] = "Items"; ui.tabs[1] = "Equip"; ui.tabs[2] = "Shop"; ui.tabs[3] = "Settings";
    ui.backpackTitle = "Backpack"; ui.itemDetails = "Item Details";
    ui.itemList = "Iron Sword (Wep);Steel Sword (Wep);Magic Sword (Wep);Leather Armor (Arm);Iron Armor (Arm);Health Potion (Pot);Mana Potion (Pot);Gold Bag (Misc);Key (Misc);Gem (Misc)";
    ui.selectPrompt = "Select an item";
    ui.capacityLabel = "Capacity: %d/%d"; ui.totalValue = "Total Value: %d Gold";
    ui.typeWeapon = "Weapon"; ui.typeArmor = "Armor"; ui.typePotion = "Potion"; ui.typeMisc = "Misc";
    ui.valueLabel = "Value: %d Gold";
    ui.btnUse = "Use"; ui.btnDrop = "Drop"; ui.btnSell = "Sell";
    ui.msgUseItem = "Use %s?"; ui.msgDropItem = "Drop %s?"; ui.msgSold = "Sold %s for %d gold";
    ui.equipTitle = "Equipment";
    ui.equipSlots[0] = "Helmet"; ui.equipSlots[1] = "L-Hand"; ui.equipSlots[2] = "Chest"; ui.equipSlots[3] = "R-Hand"; ui.equipSlots[4] = "Legs"; ui.equipSlots[5] = "Boots";
    ui.statsTitle = "Stats";
    ui.statAttack = "Attack: %d"; ui.statDefense = "Defense: %d"; ui.statHP = "HP: %d/%d"; ui.statMP = "MP: %d/%d";
    ui.appearanceLabel = "Appearance";
    ui.shopTitle = "Shop"; ui.btnBuy = "Buy"; ui.shopInfo = "Shop Info";
    ui.shopHint1 = "- Click to buy items"; ui.shopHint2 = "- Not enough gold = can't buy"; ui.shopHint3 = "- Restocks daily";
    ui.lblPlayerName = "Player Name:"; ui.lblVolume = "Volume:"; ui.lblDifficulty = "Difficulty:"; ui.lblTheme = "Theme:";
    ui.chkSound = "Sound Enabled"; ui.btnSave = "Save"; ui.btnReset = "Reset"; ui.msgSaved = "Settings saved!";
    ui.diffOptions = "Easy;Medium;Hard"; ui.themeOptions = "Default;Dark;Blue"; ui.lblTestValue = "Test Value:";
    ui.diffEasy = "Easy"; ui.diffMedium = "Medium"; ui.diffHard = "Hard";
}

void UnloadChineseFont(UIText& ui) {
    if (ui.hasChineseFont) {
        UnloadFont(ui.chineseFont);
        ui.hasChineseFont = false;
    }
}

void ApplyTheme(int theme, UIText& ui) {
    // Set larger text size for all controls
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24);

    switch (theme) {
        case 0:
            GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(RAYWHITE));
            GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(DARKGRAY));
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(LIGHTGRAY));
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(DARKGRAY));
            if (ui.hasChineseFont) GuiSetFont(ui.chineseFont);
            break;
        case 1:
            GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt((Color){30, 30, 30, 255}));
            GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt((Color){60, 60, 60, 255}));
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
            if (ui.hasChineseFont) GuiSetFont(ui.chineseFont);
            break;
        case 2:
            GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt((Color){240, 248, 255, 255}));
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(SKYBLUE));
            GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(DARKBLUE));
            if (ui.hasChineseFont) GuiSetFont(ui.chineseFont);
            break;
    }
}

bool GuiIconButton(Rectangle bounds, int iconId, const char* text, UIText& ui) {
    return GuiButton(bounds, TextFormat("#%03d# %s", iconId, text));
}

// Helper function to draw text with Chinese font support
void DrawChineseText(const char* text, int x, int y, int fontSize, Color color, UIText& ui) {
    if (ui.hasChineseFont && ui.chineseFont.texture.id != 0) {
        DrawTextEx(ui.chineseFont, text, (Vector2){(float)x, (float)y}, (float)fontSize * 2.0f, 2.0f, color);
    } else {
        DrawText(text, x, y, fontSize, color);
    }
}

int main() {
    const int screenWidth = 1000;
    const int screenHeight = 700;
    InitWindow(screenWidth, screenHeight, "Chapter 8: Raygui Advanced - RPG Inventory");
    SetTargetFPS(60);

    UIText uiText = {};
    LoadChineseFont(uiText);

    int activeTab = 0, theme = 0, inventoryScroll = 0, selectedItem = -1, itemCount = 10;
    int gold = 500;
    Color playerColor = BLUE;
    float volume = 0.8f, difficulty = 1.0f;
    bool soundEnabled = true, showMessage = false, nameEditMode = false, themeEditMode = false, spinnerEdit = false;
    int testValue = 50, messageResult = -1;
    char playerName[64] = "Hero";
    const char* messageText = "";
    const char* messageTitle = "";

    ApplyTheme(0, uiText);

    while (!WindowShouldClose()) {
        BeginDrawing();
        Color bgColor = (theme == 1) ? (Color){30, 30, 30, 255} : (theme == 2) ? (Color){240, 248, 255, 255} : RAYWHITE;
        ClearBackground(bgColor);

        GuiPanel((Rectangle){0, 0, (float)screenWidth, 100}, NULL);

        // Use DrawTextEx with Chinese font, or fallback to DrawText
        if (uiText.hasChineseFont) {
            DrawTextEx(uiText.chineseFont, uiText.title, (Vector2){20.0f, 10.0f}, 56.0f, 2.0f, (theme == 1) ? WHITE : DARKGRAY);
            DrawTextEx(uiText.chineseFont, TextFormat(uiText.goldLabel, gold), (Vector2){(float)(screenWidth - 280), 15.0f}, 36.0f, 2.0f, GOLD);
        } else {
            DrawText(uiText.title, 20, 10, 30, (theme == 1) ? WHITE : DARKGRAY);
            DrawText(TextFormat(uiText.goldLabel, gold), screenWidth - 200, 15, 20, GOLD);
        }

        // Tab buttons
        int tabX = 20, tabY = 60, tabWidth = 100, tabHeight = 35;
        for (int i = 0; i < 4; i++) {
            if (activeTab == i) {
                GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt((theme == 1) ? SKYBLUE : DARKBLUE));
                GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
            } else {
                GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt((theme == 1) ? DARKGRAY : LIGHTGRAY));
                GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt((theme == 1) ? WHITE : DARKGRAY));
            }
            if (GuiButton((Rectangle){(float)(tabX + i * (tabWidth + 10)), (float)tabY, (float)tabWidth, (float)tabHeight}, uiText.tabs[i])) {
                activeTab = i;
            }
        }
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt((theme == 1) ? (Color){60,60,60,255} : LIGHTGRAY));
        GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt((theme == 1) ? WHITE : DARKGRAY));
        if (uiText.hasChineseFont) GuiSetFont(uiText.chineseFont);

        // Content
        switch (activeTab) {
            case 0: {  // Items
                GuiPanel((Rectangle){20, 120, 400, 500}, uiText.backpackTitle);
                int newSelected = GuiListView((Rectangle){30, 150, 380, 400}, uiText.itemList, &inventoryScroll, &selectedItem);
                if (newSelected >= 0) selectedItem = newSelected;

                GuiPanel((Rectangle){440, 120, 540, 400}, uiText.itemDetails);
                if (selectedItem >= 0 && selectedItem < itemCount) {
                    Item& item = items[selectedItem];
                    DrawRectangle(460, 150, 80, 80, item.color);
                    DrawRectangleLines(460, 150, 80, 80, DARKGRAY);
                    DrawChineseText(item.name, 560, 150, 24, (theme == 1) ? WHITE : DARKGRAY, uiText);
                    const char* typeStr = (item.type == ITEM_WEAPON) ? uiText.typeWeapon : (item.type == ITEM_ARMOR) ? uiText.typeArmor : (item.type == ITEM_POTION) ? uiText.typePotion : uiText.typeMisc;
                    DrawChineseText(TextFormat("Type: %s", typeStr), 560, 180, 18, GRAY, uiText);
                    DrawChineseText(TextFormat(uiText.valueLabel, item.value), 560, 205, 18, GOLD, uiText);

                    if (GuiIconButton((Rectangle){560, 250, 120, 35}, 147, uiText.btnUse, uiText)) {
                        messageTitle = uiText.btnUse; messageText = TextFormat(uiText.msgUseItem, item.name); showMessage = true;
                    }
                    if (GuiIconButton((Rectangle){560, 295, 120, 35}, 142, uiText.btnDrop, uiText)) {
                        messageTitle = uiText.btnDrop; messageText = TextFormat(uiText.msgDropItem, item.name); showMessage = true;
                    }
                    if (GuiIconButton((Rectangle){700, 250, 120, 35}, 179, uiText.btnSell, uiText)) {
                        gold += item.value / 2; messageTitle = uiText.btnSell; messageText = TextFormat(uiText.msgSold, item.name, item.value / 2); showMessage = true;
                    }
                } else {
                    if (uiText.hasChineseFont) {
                        DrawTextEx(uiText.chineseFont, uiText.selectPrompt, (Vector2){560.0f, 150.0f}, 20.0f, 2.0f, GRAY);
                    } else {
                        DrawText(uiText.selectPrompt, 560, 150, 20, GRAY);
                    }
                }
                GuiPanel((Rectangle){440, 530, 540, 90}, NULL);
                DrawChineseText(TextFormat(uiText.capacityLabel, itemCount, 20), 460, 550, 18, (theme == 1) ? WHITE : DARKGRAY, uiText);
                DrawChineseText(TextFormat(uiText.totalValue, 2000), 460, 575, 18, GOLD, uiText);
                break;
            }

            case 1: {  // Equipment
                GuiPanel((Rectangle){20, 120, 960, 500}, uiText.equipTitle);
                Rectangle slots[] = {{350, 150, 80, 80}, {250, 250, 80, 80}, {350, 250, 80, 80}, {450, 250, 80, 80}, {350, 350, 80, 80}, {350, 450, 80, 80}};
                for (int i = 0; i < 6; i++) {
                    Color slotColor = (i == 2) ? SKYBLUE : LIGHTGRAY;
                    GuiPanel((Rectangle){slots[i].x - 5, slots[i].y - 25, slots[i].width + 10, slots[i].height + 35}, uiText.equipSlots[i]);
                    DrawRectangleRec(slots[i], slotColor);
                    DrawRectangleLinesEx(slots[i], 2, DARKGRAY);
                    if (i == 2) DrawChineseText("Iron", (int)slots[i].x + 15, (int)slots[i].y + 30, 16, DARKGRAY, uiText);
                }
                GuiPanel((Rectangle){600, 150, 300, 300}, uiText.statsTitle);
                DrawChineseText(TextFormat(uiText.statAttack, 25), 620, 180, 18, RED, uiText);
                DrawChineseText(TextFormat(uiText.statDefense, 40), 620, 205, 18, BLUE, uiText);
                DrawChineseText(TextFormat(uiText.statHP, 100, 100), 620, 230, 18, GREEN, uiText);
                DrawChineseText(TextFormat(uiText.statMP, 50, 50), 620, 255, 18, PURPLE, uiText);
                GuiColorPicker((Rectangle){620, 300, 150, 150}, NULL, &playerColor);
                DrawChineseText(uiText.appearanceLabel, 790, 320, 16, (theme == 1) ? WHITE : DARKGRAY, uiText);
                break;
            }

            case 2: {  // Shop
                GuiPanel((Rectangle){20, 120, 600, 500}, uiText.shopTitle);
                int shopY = 150;
                for (int i = 0; i < 5; i++) {
                    Item& item = items[i + 2];
                    Color rowColor1 = (theme == 1 ? (Color){40,40,40,255} : (Color){245,245,245,255});
                    Color rowColor2 = (theme == 1 ? (Color){50,50,50,255} : WHITE);
                    DrawRectangle(30, shopY, 580, 80, (i % 2 == 0) ? rowColor1 : rowColor2);
                    DrawRectangle(40, shopY + 10, 60, 60, item.color);
                    DrawChineseText(item.name, 120, shopY + 15, 20, (theme == 1) ? WHITE : DARKGRAY, uiText);
                    DrawChineseText(TextFormat("%d Gold", item.value), 120, shopY + 45, 16, GOLD, uiText);
                    bool canAfford = gold >= item.value;
                    if (!canAfford) GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(GRAY));
                    if (GuiButton((Rectangle){500, (float)(shopY + 20), 100, 40}, uiText.btnBuy)) {
                        if (canAfford) { gold -= item.value; messageTitle = uiText.btnBuy; messageText = TextFormat("Bought %s", item.name); showMessage = true; }
                    }
                    if (!canAfford) GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
                    if (uiText.hasChineseFont) GuiSetFont(uiText.chineseFont);
                    shopY += 90;
                }
                GuiPanel((Rectangle){640, 120, 340, 200}, uiText.shopInfo);
                DrawChineseText(uiText.shopHint1, 660, 150, 16, (theme == 1) ? WHITE : DARKGRAY, uiText);
                DrawChineseText(uiText.shopHint2, 660, 175, 16, (theme == 1) ? WHITE : DARKGRAY, uiText);
                DrawChineseText(uiText.shopHint3, 660, 200, 16, (theme == 1) ? WHITE : DARKGRAY, uiText);
                break;
            }

            case 3: {  // Settings
                int sx = 50, sy = 150, lw = 150;
                GuiLabel((Rectangle){(float)sx, (float)sy, (float)lw, 24}, uiText.lblPlayerName);
                if (GuiTextBox((Rectangle){(float)(sx + lw), (float)sy, 200, 24}, playerName, 64, nameEditMode)) nameEditMode = !nameEditMode;
                sy += 50;
                GuiLabel((Rectangle){(float)sx, (float)sy, (float)lw, 24}, uiText.lblVolume);
                GuiSlider((Rectangle){(float)(sx + lw), (float)(sy + 5), 200, 15}, NULL, TextFormat("%d%%", (int)(volume * 100)), &volume, 0.0f, 1.0f);
                sy += 50;
                GuiCheckBox((Rectangle){(float)sx, (float)sy, 20, 20}, uiText.chkSound, &soundEnabled);
                sy += 50;
                GuiLabel((Rectangle){(float)sx, (float)sy, (float)lw, 24}, uiText.lblDifficulty);
                GuiSliderBar((Rectangle){(float)(sx + lw), (float)(sy + 5), 200, 15}, uiText.diffEasy, uiText.diffHard, &difficulty, 0.0f, 2.0f);
                int di = (int)(difficulty + 0.5f);
                DrawChineseText((di == 0) ? uiText.diffEasy : (di == 1) ? uiText.diffMedium : uiText.diffHard, sx + lw + 210, sy, 16, (theme == 1) ? WHITE : DARKGRAY, uiText);
                sy += 60;
                GuiLabel((Rectangle){(float)sx, (float)sy, (float)lw, 24}, uiText.lblTheme);
                int newTheme = theme;
                if (GuiDropdownBox((Rectangle){(float)(sx + lw), (float)sy, 150, 30}, uiText.themeOptions, &newTheme, themeEditMode)) themeEditMode = !themeEditMode;
                if (newTheme != theme) { theme = newTheme; ApplyTheme(theme, uiText); }
                sy += 80;
                GuiLabel((Rectangle){(float)sx, (float)sy, (float)lw, 24}, uiText.lblTestValue);
                testValue = GuiSpinner((Rectangle){(float)(sx + lw), (float)sy, 120, 24}, NULL, &testValue, 0, 999, spinnerEdit);
                sy += 60;
                if (GuiButton((Rectangle){(float)sx, (float)sy, 150, 40}, uiText.btnSave)) {
                    messageTitle = uiText.btnSave; messageText = uiText.msgSaved; showMessage = true;
                }
                if (GuiButton((Rectangle){(float)(sx + 170), (float)sy, 150, 40}, uiText.btnReset)) {
                    volume = 0.8f; soundEnabled = true; difficulty = 1.0f; theme = 0; ApplyTheme(0, uiText);
                }
                break;
            }
        }

        if (showMessage) {
            messageResult = GuiMessageBox((Rectangle){screenWidth/2 - 150.0f, screenHeight/2 - 75.0f, 300, 150}, messageTitle, messageText, "OK;Cancel");
            if (messageResult >= 0) showMessage = false;
        }

        EndDrawing();
    }

    UnloadChineseFont(uiText);
    CloseWindow();
    return 0;
}
