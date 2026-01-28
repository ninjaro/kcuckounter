#ifndef KCUCKOUNTER_HELPERS_THEME_SETTINGS_HPP
#define KCUCKOUNTER_HELPERS_THEME_SETTINGS_HPP

#include <QColor>

class theme_settings {
public:
    static void set_base_color(const QColor& color);
    static QColor base_color();
    static QColor table_color();
    static QColor panel_color();
    static QColor slot_fill_color();
    static QColor slot_border_color();
    static QColor slot_border_selected_color();
};

#endif // KCUCKOUNTER_HELPERS_THEME_SETTINGS_HPP
