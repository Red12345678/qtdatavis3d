/****************************************************************************
**
** Copyright (C) 2014 Digia Plc
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

#include "datasource.h"
#include <QtQuick/QQuickView>
#include <QtQuick/QQuickItem>
#include <QDebug>
#include <qmath.h>

using namespace QtDataVisualization;

Q_DECLARE_METATYPE(QSurface3DSeries *)

DataSource::DataSource(QQuickView *appViewer, QObject *parent) :
    QObject(parent),
    m_appViewer(appViewer),
    m_index(-1),
    m_resetArray(0)
{
    qRegisterMetaType<QSurface3DSeries *>();
}

DataSource::~DataSource()
{
    clearData();
}

//! [0]
void DataSource::generateData(int cacheCount, int rowCount, int columnCount,
                              float xMin, float xMax, float yMin, float yMax,
                              float zMin, float zMax)
{
    clearData();

    // Re-create the cache array
    m_data.resize(cacheCount);
    for (int i(0); i < cacheCount; i++) {
        QSurfaceDataArray &array = m_data[i];
        array.reserve(rowCount);
        for (int j(0); j < rowCount; j++)
            array.append(new QSurfaceDataRow(columnCount));
    }

    float xRange = xMax - xMin;
    float yRange = yMax - yMin;
    float zRange = zMax - zMin;

    // Populate caches
    for (int i(0); i < cacheCount; i++) {
        QSurfaceDataArray &cache = m_data[i];
        for (int j(0); j < rowCount; j++) {
            QSurfaceDataRow &row = *(cache[j]);
            for (int k(0); k < columnCount; k++) {
                float colMod = (float(k)) / float(columnCount - 1.0f);
                float rowMod = (float(j)) / float(rowCount - 1.0f);
                float xRangeMod = xRange * colMod;
                float yRangeMod = yRange * rowMod;
                float zRangeMod = zRange * rowMod;
                float x = xRangeMod + xMin;
                float y = (((float(qSin(M_PI * rowMod / 2.0) + 1.0)))
                           + ((float(qSin(M_PI * rowMod * colMod * 5.0) + 1.0))))
                              * yRangeMod * 0.2f +
                           + (0.15f * float(rand()) / float(RAND_MAX)) * yRangeMod;
                float z = zRangeMod + zMin;
                row[k] = QVector3D(x, y, z);
            }
        }
    }
}
//! [0]

//! [1]
void DataSource::update(QSurface3DSeries *series)
{
    if (series) {
        // Each iteration uses data from a different cached array
        m_index++;
        if (m_index > m_data.count() - 1)
            m_index = 0;

        QSurfaceDataArray array = m_data.at(m_index);
        int newRowCount = array.size();
        int newColumnCount = array.at(0)->size();

        // If the first time or the dimensions of the cache array have changed,
        // reconstruct the reset array
        if (m_resetArray || series->dataProxy()->rowCount() != newRowCount
                || series->dataProxy()->columnCount() != newColumnCount) {
            m_resetArray = new QSurfaceDataArray();
            m_resetArray->reserve(newRowCount);
            for (int i(0); i < newRowCount; i++)
                m_resetArray->append(new QSurfaceDataRow(newColumnCount));
        }

        // Copy items from our cache to the reset array
        for (int i(0); i < newRowCount; i++) {
            const QSurfaceDataRow &sourceRow = *(array.at(i));
            QSurfaceDataRow &row = *(*m_resetArray)[i];
            for (int j(0); j < newColumnCount; j++)
                row[j].setPosition(sourceRow.at(j).position());
        }

        // Notify the proxy that data has changed
        series->dataProxy()->resetArray(m_resetArray);
    }
}
//! [1]

//! [2]
QString DataSource::selectionLabel(QSurface3DSeries *series, QValue3DAxis *axisX,
                                   QValue3DAxis *axisY, QValue3DAxis *axisZ)
{
    QString label;

    if (series && series->selectedPoint() != QSurface3DSeries::invalidSelectionPosition()) {
        const QSurfaceDataItem *item = series->dataProxy()->itemAt(series->selectedPoint());
        QString x;
        QString y;
        QString z;
        x.sprintf(axisX->labelFormat().toUtf8().constData(), int(item->x()));
        y.sprintf(axisY->labelFormat().toUtf8().constData(), int(item->y()));
        z.sprintf(axisZ->labelFormat().toUtf8().constData(), int(item->z()));
        label = QStringLiteral("%1, %3: %2").arg(x).arg(y).arg(z);
    } else {
        label = QStringLiteral("No selection");
    }

    return label;
}
//! [2]

void DataSource::clearData()
{
    for (int i(0); i < m_data.size(); i++) {
        QSurfaceDataArray &array = m_data[i];
        for (int j(0); j < array.size(); j++)
            delete array[j];
        array.clear();
    }
}
