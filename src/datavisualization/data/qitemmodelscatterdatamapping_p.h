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

#ifndef QITEMMODELSCATTERDATAMAPPING_P_H
#define QITEMMODELSCATTERDATAMAPPING_P_H

#include "qitemmodelscatterdatamapping.h"
#include "qabstractdatamapping_p.h"

QT_DATAVISUALIZATION_BEGIN_NAMESPACE

class QItemModelScatterDataMappingPrivate : public QAbstractDataMappingPrivate
{
    Q_OBJECT
public:
    QItemModelScatterDataMappingPrivate(QItemModelScatterDataMapping *q);
    virtual ~QItemModelScatterDataMappingPrivate();

private:
    //QString m_labelRole;
    QString m_xPosRole;
    QString m_yPosRole;
    QString m_zPosRole;
    //QString m_valueRole;

    friend class QItemModelScatterDataMapping;
};

QT_DATAVISUALIZATION_END_NAMESPACE

#endif
