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

#include "compat/config_shim.hpp"

#ifdef KC_KDE
#else

KConfigGroup::KConfigGroup(const QString& rootGroup)
    : m_root(rootGroup) { }

KConfigGroup KConfigGroup::group(const QString& child) const {
    return KConfigGroup(m_root.isEmpty() ? child : (m_root + '/' + child));
}

QStringList KConfigGroup::groupList() const {
    QSettings s;
    s.beginGroup(m_root);
    const auto g = s.childGroups();
    s.endGroup();
    return g;
}

QString KConfigGroup::readEntry(const QString& key, const QString& def) const {
    QSettings s;
    s.beginGroup(m_root);
    const QVariant v = s.value(key);
    s.endGroup();
    return v.isValid() ? v.toString() : def;
}

QString KConfigGroup::readEntry(const QString& key, const char* def) const {
    return readEntry(key, QString::fromUtf8(def));
}

QList<qint32>
KConfigGroup::readEntry(const QString& key, const QList<qint32>& def) const {
    QSettings s;
    s.beginGroup(m_root);
    const QVariant v = s.value(key);
    s.endGroup();
    if (!v.isValid()) {
        return def;
    }
    QList<qint32> out;
    for (const QVariant& x : v.toList()) {
        out.push_back(x.toInt());
    }
    return out;
}

void KConfigGroup::writeEntry(
    const QString& key, const QList<qint32>& value
) const {
    QVariantList lst;
    lst.reserve(value.size());
    for (qint32 v : value) {
        lst.push_back(v);
    }
    QSettings s;
    s.beginGroup(m_root);
    s.setValue(key, lst);
    s.endGroup();
}

void KConfigGroup::ConfigProxy::sync() { QSettings().sync(); }

#endif