/****************************************************************************
**
** Copyright (C) 2013 Digia Plc
** All rights reserved.
** For any questions to Digia, please use contact form at http://qt.digia.com
**
** This file is part of the QtDataVisualization module.
**
** Licensees holding valid Qt Enterprise licenses may use this file in
** accordance with the Qt Enterprise License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.
**
** If you have questions regarding the use of this file, please use
** contact form at http://qt.digia.com
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QtDataVisualization API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef DECLARATIVESURFACE_P_H
#define DECLARATIVESURFACE_P_H

#include "datavisualizationglobal_p.h"
#include "abstractdeclarative_p.h"
#include "surface3dcontroller_p.h"
#include "declarativesurface_p.h"
#include "q3dvalueaxis.h"
#include "qsurfacedataproxy.h"
#include "colorgradient_p.h"
#include "qsurface3dseries.h"

#include <QAbstractItemModel>
#include <QQuickItem>
#include <QObject>
#include <QQuickWindow>

QT_DATAVISUALIZATION_BEGIN_NAMESPACE

class DeclarativeSurface : public AbstractDeclarative
{
    Q_OBJECT
    Q_PROPERTY(Q3DValueAxis *axisX READ axisX WRITE setAxisX)
    Q_PROPERTY(Q3DValueAxis *axisY READ axisY WRITE setAxisY)
    Q_PROPERTY(Q3DValueAxis *axisZ READ axisZ WRITE setAxisZ)
    Q_PROPERTY(bool smoothSurfaceEnabled READ isSmoothSurfaceEnabled WRITE setSmoothSurfaceEnabled NOTIFY smoothSurfaceEnabledChanged)
    Q_PROPERTY(bool surfaceGridEnabled READ isSurfaceGridEnabled WRITE setSurfaceGridEnabled NOTIFY surfaceGridEnabledChanged)
    Q_PROPERTY(ColorGradient *gradient READ gradient WRITE setGradient)
    Q_PROPERTY(QPointF selectedPoint READ selectedPoint WRITE setSelectedPoint NOTIFY selectedPointChanged)
    Q_PROPERTY(QQmlListProperty<QSurface3DSeries> seriesList READ seriesList)
    Q_CLASSINFO("DefaultProperty", "seriesList")

public:
    explicit DeclarativeSurface(QQuickItem *parent = 0);
    ~DeclarativeSurface();

    Q3DValueAxis *axisX() const;
    void setAxisX(Q3DValueAxis *axis);
    Q3DValueAxis *axisY() const;
    void setAxisY(Q3DValueAxis *axis);
    Q3DValueAxis *axisZ() const;
    void setAxisZ(Q3DValueAxis *axis);

    void setSmoothSurfaceEnabled(bool enabled);
    bool isSmoothSurfaceEnabled() const;

    void setSurfaceGridEnabled(bool enabled);
    bool isSurfaceGridEnabled() const;

    void setGradient(ColorGradient *gradient);
    ColorGradient *gradient() const;

    void setSelectedPoint(const QPointF &position);
    QPointF selectedPoint() const;

    QQmlListProperty<QSurface3DSeries> seriesList();
    static void appendSeriesFunc(QQmlListProperty<QSurface3DSeries> *list, QSurface3DSeries *series);
    static int countSeriesFunc(QQmlListProperty<QSurface3DSeries> *list);
    static QSurface3DSeries *atSeriesFunc(QQmlListProperty<QSurface3DSeries> *list, int index);
    static void clearSeriesFunc(QQmlListProperty<QSurface3DSeries> *list);
    Q_INVOKABLE void addSeries(QSurface3DSeries *series);
    Q_INVOKABLE void removeSeries(QSurface3DSeries *series);

signals:
    void surfaceVisibleChanged(bool visible);
    void smoothSurfaceEnabledChanged(bool enabled);
    void surfaceGridEnabledChanged(bool visible);
    void selectedPointChanged(QPoint position);

protected:
    void handleGradientUpdate();

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

private:
    Surface3DController *m_shared;

    void setControllerGradient(const ColorGradient &gradient);

    QSize m_initialisedSize;
    ColorGradient *m_gradient; // Not owned
};

QT_DATAVISUALIZATION_END_NAMESPACE

#endif
