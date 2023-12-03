/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Nicolas Arnaud-Cormos <nicolas.arnaud-cormos@kdab.com>

  SPDX-License-Identifier: MIT
*/

#include <QtCore>

#include <unordered_set>
#include <unordered_map>

int main(int argc, char *argv[])
{
    auto mapString =
        std::map<int, QString>{{10, QStringLiteral("one")}, {20, QStringLiteral("two")}, {30, QStringLiteral("three")}};
    auto qmapString =
        QMap<int, QString>{{10, QStringLiteral("one")}, {20, QStringLiteral("two")}, {30, QStringLiteral("three")}};

    auto hashString = std::unordered_map<int, QString>{
        {10, QStringLiteral("one")}, {20, QStringLiteral("two")}, {30, QStringLiteral("three")}};
    auto qhashString =
        QHash<int, QString>{{10, QStringLiteral("one")}, {20, QStringLiteral("two")}, {30, QStringLiteral("three")}};

    auto multihashString = std::unordered_multimap<int, QString>{{10, QStringLiteral("one")},
                                                                 {10, QStringLiteral("two")},
                                                                 {10, QStringLiteral("three")},
                                                                 {20, QStringLiteral("four")}};
    auto qmultihashString = QMultiHash<int, QString>{{10, QStringLiteral("one")},
                                                     {10, QStringLiteral("two")},
                                                     {10, QStringLiteral("three")},
                                                     {20, QStringLiteral("four")}};

    auto setString = std::unordered_set<std::string>{"one", "two", "three", "four"};
    auto qsetString =
        QSet<QString>{QStringLiteral("one"), QStringLiteral("two"), QStringLiteral("three"), QStringLiteral("four")};

    QVariant v_null;
    QVariant v_bool = true;
    QVariant v_int = -10;
    QVariant v_uint = 10u;
    QVariant v_longlong = -100000ll;
    QVariant v_ulonglong = 100000ull;
    QVariant v_float = 3.1415;
    QVariant v_double = 1.602'176'634e-19;
    QVariant v_char = QVariant::fromValue(static_cast<char>('V'));

    QVariant v_qchar = QChar(u'V');
    QVariant v_qstring = QStringLiteral("Hello World!");
    QVariant v_qbytearray = QByteArray("Hello World!");
    QVariant v_qdate = QDate::currentDate();
    QVariant v_qtime = QTime::currentTime();
    QVariant v_qdatetime = QDateTime::currentDateTime();            // NOK
    QVariant v_qurl = QUrl(QStringLiteral("https://www.kdab.com")); // NOK
    QVariant v_qlocale = QLocale();                                 // NOK
    QVariant v_qrect = QRect(5, 5, 42, 42);
    QVariant v_qrectf = QRectF(5., 5., 4.2, 4.2);
    QVariant v_size = QSize(5, 42);
    QVariant v_sizef = QSizeF(5., 4.2);
    QVariant v_qline = QLine(5, 5, 42, 42);
    QVariant v_qlinef = QLineF(5., 5., 4.2, 4.2);
    QVariant v_qpoint = QPoint(5, 42);
    QVariant v_qpointf = QPointF(5., 4.2);

    auto v_map = QVariantMap{
        {QStringLiteral("string"), v_qstring}, {QStringLiteral("double"), v_double}, {QStringLiteral("rect"), v_qrect}};
    auto v_list = QVariantList{v_qstring, v_double, v_qrect};
    auto v_hash = QVariantHash{
        {QStringLiteral("string"), v_qstring}, {QStringLiteral("double"), v_double}, {QStringLiteral("rect"), v_qrect}};
    auto v_stringlist = QStringList{QStringLiteral("one"), QStringLiteral("two"), QStringLiteral("three")};

    return 0;
}
