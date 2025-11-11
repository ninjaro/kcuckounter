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

#ifndef KCUCKOUNTER_CONFIG_SHIM_HPP
#define KCUCKOUNTER_CONFIG_SHIM_HPP

#ifdef KC_KDE
#include <KConfigGroup>
#include <KSharedConfig>
#else
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

class KConfigGroup {
public:
    explicit KConfigGroup(const QString& rootGroup = QString());

    KConfigGroup group(const QString& child) const;

    QStringList groupList() const;

    QString readEntry(const QString& key, const QString& def) const;

    QString readEntry(const QString& key, const char* def) const;

    template <typename T> T readEntry(const QString& key, const T& def) const;

    QList<qint32> readEntry(const QString& key, const QList<qint32>& def) const;

    template <typename T> void writeEntry(const QString& key, const T& value);

    void writeEntry(const QString& key, const QList<qint32>& value) const;

    struct ConfigProxy {
        static void sync();
    };

    static ConfigProxy* config() {
        static ConfigProxy p;
        return &p;
    }

private:
    QString m_root;
};

template <typename T>
T KConfigGroup::readEntry(const QString& key, const T& def) const {
    QSettings s;
    s.beginGroup(m_root);
    const QVariant v = s.value(key);
    s.endGroup();
    if (!v.isValid()) {
        return def;
    }
    return v.value<T>();
}

template <typename T>
void KConfigGroup::writeEntry(const QString& key, const T& value) {
    QSettings s;
    s.beginGroup(m_root);
    s.setValue(key, QVariant::fromValue(value));
    s.endGroup();
}

struct KSharedConfig {
    static void* openConfig() { return nullptr; }
};
#endif

#endif // KCUCKOUNTER_CONFIG_SHIM_HPP
