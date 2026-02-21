#include "ui_theme.h"

namespace GitGUI {

Theme Theme::Default() {
    Theme theme;
    theme.background = {30, 30, 40, 255};
    theme.panelBg = {40, 40, 55, 255};
    theme.panelBorder = {60, 60, 80, 255};
    theme.textPrimary = {240, 240, 245, 255};
    theme.textSecondary = {180, 180, 190, 255};
    theme.primary = {70, 140, 240, 255};
    theme.primaryHover = {90, 160, 255, 255};
    theme.primaryActive = {50, 120, 220, 255};
    theme.secondary = {80, 80, 95, 255};
    theme.secondaryHover = {100, 100, 115, 255};
    theme.success = {80, 200, 120, 255};
    theme.warning = {250, 180, 60, 255};
    theme.error = {240, 80, 80, 255};
    theme.accent = {160, 90, 240, 255};
    theme.successLight = {100, 220, 140, 255};
    theme.border = {70, 70, 90, 255};
    return theme;
}

Theme Theme::Dark() {
    Theme theme;
    theme.background = {20, 20, 25, 255};
    theme.panelBg = {30, 30, 38, 255};
    theme.panelBorder = {50, 50, 65, 255};
    theme.textPrimary = {220, 220, 230, 255};
    theme.textSecondary = {150, 150, 165, 255};
    theme.primary = {60, 130, 230, 255};
    theme.primaryHover = {80, 150, 255, 255};
    theme.primaryActive = {40, 110, 210, 255};
    theme.secondary = {70, 70, 85, 255};
    theme.secondaryHover = {90, 90, 105, 255};
    theme.success = {70, 190, 110, 255};
    theme.warning = {240, 170, 50, 255};
    theme.error = {230, 70, 70, 255};
    theme.accent = {150, 80, 230, 255};
    theme.successLight = {90, 210, 130, 255};
    theme.border = {60, 60, 75, 255};
    return theme;
}

} // namespace GitGUI
