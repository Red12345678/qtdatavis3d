/******************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Data Visualization module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#ifndef QTOUCH3DINPUTHANDLER_P_H
#define QTOUCH3DINPUTHANDLER_P_H

#include "q3dinputhandler_p.h"
#include "qtouch3dinputhandler.h"

QT_FORWARD_DECLARE_CLASS(QTimer)

QT_BEGIN_NAMESPACE_DATAVISUALIZATION

class QAbstract3DInputHandler;

class QTouch3DInputHandlerPrivate : public Q3DInputHandlerPrivate
{
    Q_OBJECT

public:
    QTouch3DInputHandlerPrivate(QTouch3DInputHandler *q);
    ~QTouch3DInputHandlerPrivate();

    void handlePinchZoom(float distance, const QPoint &pos);
    void handleTapAndHold();
    void handleSelection(const QPointF &position);
    void handleRotation(const QPointF &position);

private:
    QTouch3DInputHandler *q_ptr;
public:
    QTimer *m_holdTimer;
    QAbstract3DInputHandlerPrivate::InputState m_inputState;
    QPointF m_startHoldPos;
    QPointF m_touchHoldPos;
};

QT_END_NAMESPACE_DATAVISUALIZATION

#endif
