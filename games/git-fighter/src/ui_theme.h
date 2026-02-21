#pragma once
#include <raylib.h>

namespace GitGUI {

// Modern UI color theme
struct Theme {
    Color background;
    Color panelBg;
    Color panelBorder;
    Color textPrimary;
    Color textSecondary;
    Color primary;
    Color primaryHover;
    Color primaryActive;
    Color secondary;
    Color secondaryHover;
    Color success;
    Color warning;
    Color error;
    Color accent;
    Color successLight;
    Color border;
    
    static Theme Default();
    static Theme Dark();
};

} // namespace GitGUI
