/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Yaroslav Riabtsev <yaroslav.riabtsev@rwth-aachen.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef KCUCKOUNTER_I18N_SHIM_HPP
#define KCUCKOUNTER_I18N_SHIM_HPP

#ifdef KC_KDE
#include <KLocalizedString>
#else
#include <QObject>
#include <QString>

inline QString qt_i18n_arg(QString s) { return s; }

template <typename T, typename... Rest>
inline QString qt_i18n_arg(QString s, T&& t, Rest&&... rest) {
    return qt_i18n_arg(s.arg(std::forward<T>(t)), std::forward<Rest>(rest)...);
}

inline QString i18n(const char* text) { return QObject::tr(text); }

template <typename... Args>
inline QString i18n(const char* text, Args&&... args) {
    return qt_i18n_arg(QObject::tr(text), std::forward<Args>(args)...);
}
#endif
#endif // KCUCKOUNTER_I18N_SHIM_HPP
