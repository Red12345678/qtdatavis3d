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

#include "baritemmodelhandler_p.h"
#include "qitemmodelbardatamapping_p.h"

QT_DATAVISUALIZATION_BEGIN_NAMESPACE

BarItemModelHandler::BarItemModelHandler(QItemModelBarDataProxy *proxy, QObject *parent)
    : AbstractItemModelHandler(parent),
      m_proxy(proxy)
{
}

BarItemModelHandler::~BarItemModelHandler()
{
}

// Resolve entire item model into QBarDataArray.
void BarItemModelHandler::resolveModel()
{
    QItemModelBarDataMapping *mapping = static_cast<QItemModelBarDataMapping *>(m_activeMapping);
    if (m_itemModel.isNull() || !mapping) {
        m_proxy->resetArray(0);
        return;
    }

    if (!mapping->useModelCategories()
            && (mapping->rowRole().isEmpty() || mapping->columnRole().isEmpty())) {
        m_proxy->resetArray(0);
        return;
    }

    QStringList rowLabels;
    QStringList columnLabels;

    QBarDataArray *newProxyArray = new QBarDataArray;
    QHash<int, QByteArray> roleHash = m_itemModel->roleNames();

    // Default to display role if no mapping
    int valueRole = roleHash.key(mapping->valueRole().toLatin1(), Qt::DisplayRole);
    int rowCount = m_itemModel->rowCount();
    int columnCount = m_itemModel->columnCount();

    if (mapping->useModelCategories()) {
        for (int i = 0; i < rowCount; i++) {
            QBarDataRow *newProxyRow = new QBarDataRow(columnCount);
            for (int j = 0; j < columnCount; j++)
                (*newProxyRow)[j].setValue(m_itemModel->index(i, j).data(valueRole).toReal());
            newProxyArray->append(newProxyRow);
        }
        // Generate labels from headers if using model rows/columns
        for (int i = 0; i < rowCount; i++)
            rowLabels << m_itemModel->headerData(i, Qt::Vertical).toString();
        for (int i = 0; i < columnCount; i++)
            columnLabels << m_itemModel->headerData(i, Qt::Horizontal).toString();
    } else {
        int rowRole = roleHash.key(mapping->rowRole().toLatin1());
        int columnRole = roleHash.key(mapping->columnRole().toLatin1());

        bool generateRows = mapping->autoRowCategories();
        bool generateColumns = mapping->autoColumnCategories();
        QStringList rowList;
        QStringList columnList;
        // For detecting duplicates in categories generation, using QHashes should be faster than
        // simple QStringList::contains() check.
        QHash<QString, bool> rowListHash;
        QHash<QString, bool> columnListHash;

        // Sort values into rows and columns
        typedef QHash<QString, qreal> ColumnValueMap;
        QHash <QString, ColumnValueMap> itemValueMap;
        for (int i = 0; i < rowCount; i++) {
            for (int j = 0; j < columnCount; j++) {
                QModelIndex index = m_itemModel->index(i, j);
                QString rowRoleStr = index.data(rowRole).toString();
                QString columnRoleStr = index.data(columnRole).toString();
                itemValueMap[rowRoleStr][columnRoleStr] = index.data(valueRole).toReal();
                if (generateRows && !rowListHash.value(rowRoleStr, false)) {
                    rowListHash.insert(rowRoleStr, true);
                    rowList << rowRoleStr;
                }
                if (generateColumns && !columnListHash.value(columnRoleStr, false)) {
                    columnListHash.insert(columnRoleStr, true);
                    columnList << columnRoleStr;
                }
            }
        }

        if (generateRows)
            mapping->dptr()->m_rowCategories = rowList;
        else
            rowList = mapping->rowCategories();

        if (generateColumns)
            mapping->dptr()->m_columnCategories = columnList;
        else
            columnList = mapping->columnCategories();

        // Create new data array from itemValueMap
        foreach (QString rowKey, rowList) {
            QBarDataRow *newProxyRow = new QBarDataRow(columnList.size());
            for (int i = 0; i < columnList.size(); i++)
                (*newProxyRow)[i].setValue(itemValueMap[rowKey][columnList.at(i)]);
            newProxyArray->append(newProxyRow);
        }

        rowLabels = rowList;
        columnLabels = columnList;
    }

    m_proxy->resetArray(newProxyArray, rowLabels, columnLabels);
}

QT_DATAVISUALIZATION_END_NAMESPACE
