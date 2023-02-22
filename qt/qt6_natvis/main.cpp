/****************************************************************************
**                               GPLv3 License
**
** Copyright (C) 2023 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Author: Nicolas Arnaud-Cormos <nicolas.arnaud-cormos@kdab.com>
**
** This file is part of KDToolBox (https://github.com/KDAB/KDToolBox).
**
** qt6_natvis is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** qt6_natvis is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with qt6_natvis.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#include <QtCore>

#include <unordered_set>

int main(int argc, char *argv[])
{
    auto mapString = std::map<int, QString>{{10, "one"}, {20, "two"}, {30, "three"}};
    auto qmapString = QMap<int, QString>{{10, "one"}, {20, "two"}, {30, "three"}};

    auto hashString = std::unordered_map<int, QString>{{10, "one"}, {20, "two"}, {30, "three"}};
    auto qhashString = QHash<int, QString>{{10, "one"}, {20, "two"}, {30, "three"}};

    auto multihashString = std::unordered_multimap<int, QString>{{10, "one"}, {10, "two"}, {10, "three"}, {20, "four"}};
    auto qmultihashString = QMultiHash<int, QString>{{10, "one"}, {10, "two"}, {10, "three"}, {20, "four"}};

    auto setString = std::unordered_set<std::string>{"one", "two", "three", "four"};
    auto qsetString = QSet<QString>{"one", "two", "three", "four"};

    QVariant v_null;
    QVariant v_bool = true;
    QVariant v_int = -10;
    QVariant v_uint = 10u;
    QVariant v_longlong = -100000ll;
    QVariant v_ulonglong = 100000ull;
    QVariant v_float = 3.1415;
    QVariant v_double = 1.602'176'634e-19;
    QVariant v_char = QVariant::fromValue(static_cast<char>('V'));

    QVariant v_qchar = QChar('V');
    QVariant v_qstring = "Hello World!";
    QVariant v_qbytearray = QByteArray("Hello World!");
    QVariant v_qdate = QDate::currentDate();
    QVariant v_qtime = QTime::currentTime();
    QVariant v_qdatetime = QDateTime::currentDateTime(); // NOK
    QVariant v_qurl = QUrl("https://www.kdab.com");      // NOK
    QVariant v_qlocale = QLocale();                      // NOK
    QVariant v_qrect = QRect(5, 5, 42, 42);
    QVariant v_qrectf = QRectF(5., 5., 4.2, 4.2);
    QVariant v_size = QSize(5, 42);
    QVariant v_sizef = QSizeF(5., 4.2);
    QVariant v_qline = QLine(5, 5, 42, 42);
    QVariant v_qlinef = QLineF(5., 5., 4.2, 4.2);
    QVariant v_qpoint = QPoint(5, 42);
    QVariant v_qpointf = QPointF(5., 4.2);

    auto v_map = QVariantMap{{"string", v_qstring}, {"double", v_double}, {"rect", v_qrect}};
    auto v_list = QVariantList{v_qstring, v_double, v_qrect};
    auto v_hash = QVariantHash{{"string", v_qstring}, {"double", v_double}, {"rect", v_qrect}};
    auto v_stringlist = QStringList{"one", "two", "three"};

    return 0;
}
