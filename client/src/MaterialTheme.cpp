//
// Created by instellate on 2026-01-22.
//

#include "MaterialTheme.h"

#include <QApplication>
#include <QStyleHints>

#include "android.h"

MaterialTheme::MaterialTheme(QObject *parent) : QObject(parent) {
    connect(QApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &MaterialTheme::systemThemeChanged);
#ifdef Q_OS_ANDROID
    const MaterialThemeColors colors = getMaterialTheme();
    this->_accent = colors.accent;
    this->_background = colors.background;
    this->_foreground = colors.foreground;
    this->_primary = colors.primary;
#endif
}

QString MaterialTheme::accent() const { return this->_accent; }
QString MaterialTheme::background() const { return this->_background; }
QString MaterialTheme::foreground() const { return this->_foreground; }
QString MaterialTheme::primary() const { return this->_primary; }

void MaterialTheme::systemThemeChanged() {
#ifdef Q_OS_ANDROID
    const MaterialThemeColors colors = getMaterialTheme();
    this->_accent = colors.accent;
    this->_background = colors.background;
    this->_foreground = colors.foreground;
    this->_primary = colors.primary;
#endif
}
