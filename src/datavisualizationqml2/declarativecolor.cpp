/****************************************************************************
**
** Copyright (C) 2014 Digia Plc
** All rights reserved.
** For any questions to Digia, please use contact form at http://qt.io
**
** This file is part of the Qt Data Visualization module.
**
** Licensees holding valid commercial license for Qt may use this file in
** accordance with the Qt License Agreement provided with the Software
** or, alternatively, in accordance with the terms contained in a written
** agreement between you and Digia.
**
** If you have questions regarding the use of this file, please use
** contact form at http://qt.io
**
****************************************************************************/

#include "declarativecolor_p.h"

QT_BEGIN_NAMESPACE_DATAVISUALIZATION

DeclarativeColor::DeclarativeColor(QObject *parent)
    : QObject(parent)
{
}

void DeclarativeColor::setColor(const QColor &color)
{
    if (m_color != color) {
        m_color = color;
        emit colorChanged(color);
    }
}

QColor DeclarativeColor::color() const
{
    return m_color;
}

QT_END_NAMESPACE_DATAVISUALIZATION
