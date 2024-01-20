/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: MIT
*/

#ifndef KDTOOLBOX_DPIINFO_H
#define KDTOOLBOX_DPIINFO_H

#include <QFont>
#include <QGuiApplication>
#include <QScreen>
#include <QString>
#include <QStringList>

#ifdef Q_OS_WIN
#include "shellscalingapi.h"
#endif

namespace KDToolBox
{

inline QString hdpiInfo()
{
    const auto envVars = {"QT_AUTO_SCREEN_SCALE_FACTOR",
                          "QT_STYLE_OVERRIDE",
                          "QT_SCREEN_SCALE_FACTORS",
                          "QT_DEVICE_PIXEL_RATIO",
                          "QT_ENABLE_HIGHDPI_SCALING",
                          "QT_SCALE_FACTOR",
                          "QT_FONT_DPI",
                          "QT_USE_PHYSICAL_DPI"};

    const QFont font = qApp->font();
    QString result = QStringLiteral("defaultFontPx=%1; defaultFontPt=%2; app.dpr=%3; qpa=%4\n")
                         .arg(font.pixelSize())
                         .arg(font.pointSize())
                         .arg(qApp->devicePixelRatio())
                         .arg(qApp->platformName());
    for (const char *envVar : envVars)
    {
        if (qEnvironmentVariableIsSet(envVar))
        {
            result += QStringLiteral("%1=%2; ").arg(QString::fromLatin1(envVar)).arg(qEnvironmentVariable(envVar));
        }
    }

    const QList<QScreen *> screens = qApp->screens();
    for (QScreen *screen : screens)
    {
        result += QStringLiteral("\nScreen %1: %2x%3, physDpi=%4, logicalDpi=%5, dpr=%6")
                      .arg(screen->name())
                      .arg(screen->size().width())
                      .arg(screen->size().height())
                      .arg(screen->physicalDotsPerInch())
                      .arg(screen->logicalDotsPerInch())
                      .arg(screen->devicePixelRatio());
    }

#ifdef Q_OS_WIN

    PROCESS_DPI_AWARENESS pda;
    auto res = GetProcessDpiAwareness(NULL, &pda);
    if (res == S_OK)
    {
        result += QStringLiteral("\nDPI AWARENESS = %1 (%2)")
                      .arg((pda == PROCESS_DPI_UNAWARE)             ? QStringLiteral("PROCESS_DPI_UNAWARE")
                           : (pda == PROCESS_SYSTEM_DPI_AWARE)      ? QStringLiteral("PROCESS_SYSTEM_DPI_AWARE")
                           : (pda == PROCESS_PER_MONITOR_DPI_AWARE) ? QStringLiteral("PROCESS_PER_MONITOR_DPI_AWARE")
                                                                    : QStringLiteral("Unknown"))
                      .arg(pda);
    }
    else
    {
        result += "Error while retrieving DPI awareness setting";
    }

#endif

    return result;
}

}

#endif
