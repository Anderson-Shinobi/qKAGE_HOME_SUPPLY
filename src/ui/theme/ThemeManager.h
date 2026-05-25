#pragma once

#include <QString>

class ThemeManager {
public:
    static QString applicationStyleSheet();
    static QString primaryFontFamily();
    static QString accentColor();
    static QString backgroundColor();
    static QString panelColor();
    static QString primaryTextColor();
    static QString secondaryTextColor();
};
