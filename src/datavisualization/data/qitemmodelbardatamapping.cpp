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

#include "qitemmodelbardatamapping_p.h"

QT_DATAVISUALIZATION_BEGIN_NAMESPACE

/*!
 * \class QItemModelBarDataMapping
 * \inmodule QtDataVisualization
 * \brief Data model mapping for Q3DBars.
 * \since 1.0.0
 *
 * QItemModelBarDataMapping is used to map roles of QAbstractItemModel to rows, columns, and values
 * of Q3DBars. There are three ways to use QItemModelBarDataMapping:
 *
 * 1) If useModelCategories property is set to true, QItemModelBarDataMapping will map rows and
 *    columns of QAbstractItemModel to rows and columns of Q3DBars, and uses the value returned for
 *    Qt::DisplayRole as bar value by default.
 *    The value role to be used can be redefined if Qt::DisplayRole is not suitable.
 *
 * 2) For models that do not have data already neatly sorted into rows and columns, such as
 *    QAbstractListModel based models, you can define a role from the model to map for each of row,
 *    column and value.
 *
 * 3) If you do not want to include all data contained in the model, or the autogenerated rows and
 *    columns are not ordered as you wish, you can specify which rows and columns should be included
 *    and in which order by defining an explicit list of categories for either or both of rows and
 *    columns.
 *
 *    For example, assume that you have a custom QAbstractItemModel for storing various monthly values
 *    related to a business.
 *    Each item in the model has roles "year", "month", "income", and "expenses".
 *    You could do the following to display the data in a bar chart:
 *
 *    \snippet doc_src_qtdatavisualization.cpp 3
 *
 * \sa QItemModelBarDataProxy
 */

/*!
 * \qmltype BarDataMapping
 * \instantiates QItemModelBarDataMapping
 *
 * This type is used to map roles of AbstractItemModel to rows, columns, and values of Bars3D. For
 * more complete description, see QItemModelBarDataMapping.
 *
 * Usage example:
 *
 * \snippet doc_src_qmldatavisualization.cpp 4
 *
 * \sa ItemModelBarDataProxy
 */

/*!
 * \qmlproperty string BarDataMapping::rowRole
 * The row role of the mapping.
 */

/*!
 * \qmlproperty string BarDataMapping::columnRole
 * The column role of the mapping.
 */

/*!
 * \qmlproperty string BarDataMapping::valueRole
 * The value role of the mapping.
 */

/*!
 * \qmlproperty list BarDataMapping::rowCategories
 * The row categories of the mapping.
 */

/*!
 * \qmlproperty list BarDataMapping::columnCategories
 * The column categories of the mapping.
 */

/*!
 * \qmlproperty list BarDataMapping::useModelCategories
 * When set to true, the mapping ignores row and column roles and categories, and uses
 * the rows and columns from the model instead. Defaults to false.
 */

/*!
 * \qmlproperty list BarDataMapping::autoRowCategories
 * When set to true, the mapping ignores any explicitly set row categories
 * and overwrites them with automatically generated ones whenever the
 * data from model is resolved. Defaults to true.
 */

/*!
 * \qmlproperty list BarDataMapping::autoColumnCategories
 * When set to true, the mapping ignores any explicitly set column categories
 * and overwrites them with automatically generated ones whenever the
 * data from model is resolved. Defaults to true.
 */

/*!
 * Constructs QItemModelBarDataMapping with the given \a parent.
 */
QItemModelBarDataMapping::QItemModelBarDataMapping(QObject *parent)
    : QAbstractDataMapping(new QItemModelBarDataMappingPrivate(this), parent)
{
}

/*!
 * Constructs QItemModelBarDataMapping with \a valueRole and the given \a parent.
 * This constructor is meant to be used with models that have data properly sorted
 * in rows and columns already, so it also sets useModelCategories property to true.
 */
QItemModelBarDataMapping::QItemModelBarDataMapping(const QString &valueRole, QObject *parent)
    : QAbstractDataMapping(new QItemModelBarDataMappingPrivate(this), parent)
{
    dptr()->m_valueRole = valueRole;
    dptr()->m_useModelCategories = true;
}

/*!
 * Constructs QItemModelBarDataMapping with \a rowRole, \a columnRole, \a valueRole
 * and the given \a parent.
 */
QItemModelBarDataMapping::QItemModelBarDataMapping(const QString &rowRole,
                                                   const QString &columnRole,
                                                   const QString &valueRole,
                                                   QObject *parent)
    : QAbstractDataMapping(new QItemModelBarDataMappingPrivate(this), parent)
{
    dptr()->m_rowRole = rowRole;
    dptr()->m_columnRole = columnRole;
    dptr()->m_valueRole = valueRole;
}

/*!
 * Constructs QItemModelBarDataMapping with \a rowRole, \a columnRole, \a valueRole,
 * \a rowCategories, \a columnCategories and the given \a parent. This constructor
 * also sets autoRowCategories and autoColumnCategories to false.
 */
QItemModelBarDataMapping::QItemModelBarDataMapping(const QString &rowRole,
                                                   const QString &columnRole,
                                                   const QString &valueRole,
                                                   const QStringList &rowCategories,
                                                   const QStringList &columnCategories,
                                                   QObject *parent)
    : QAbstractDataMapping(new QItemModelBarDataMappingPrivate(this), parent)
{
    dptr()->m_rowRole = rowRole;
    dptr()->m_columnRole = columnRole;
    dptr()->m_valueRole = valueRole;
    dptr()->m_rowCategories = rowCategories;
    dptr()->m_columnCategories = columnCategories;
    dptr()->m_autoRowCategories = false;
    dptr()->m_autoColumnCategories = false;
}

/*!
 * Destroys QItemModelBarDataMapping.
 */
QItemModelBarDataMapping::~QItemModelBarDataMapping()
{
}

/*!
 * \property QItemModelBarDataMapping::rowRole
 *
 * Defines the row role for the mapping.
 */
void QItemModelBarDataMapping::setRowRole(const QString &role)
{
    if (dptr()->m_rowRole != role) {
        dptr()->m_rowRole = role;
        emit mappingChanged();
    }
}

QString QItemModelBarDataMapping::rowRole() const
{
    return dptrc()->m_rowRole;
}

/*!
 * \property QItemModelBarDataMapping::columnRole
 *
 * Defines the column role for the mapping.
 */
void QItemModelBarDataMapping::setColumnRole(const QString &role)
{
    if (dptr()->m_columnRole != role) {
        dptr()->m_columnRole = role;
        emit mappingChanged();
    }
}

QString QItemModelBarDataMapping::columnRole() const
{
    return dptrc()->m_columnRole;
}

/*!
 * \property QItemModelBarDataMapping::valueRole
 *
 * Defines the value role for the mapping.
 */
void QItemModelBarDataMapping::setValueRole(const QString &role)
{
    if (dptr()->m_valueRole != role) {
        dptr()->m_valueRole = role;
        emit mappingChanged();
    }
}

QString QItemModelBarDataMapping::valueRole() const
{
    return dptrc()->m_valueRole;
}

/*!
 * \property QItemModelBarDataMapping::rowCategories
 *
 * Defines the row categories for the mapping.
 */
void QItemModelBarDataMapping::setRowCategories(const QStringList &categories)
{
    if (dptr()->m_rowCategories != categories) {
        dptr()->m_rowCategories = categories;
        emit mappingChanged();
    }
}

QStringList QItemModelBarDataMapping::rowCategories() const
{
    return dptrc()->m_rowCategories;
}

/*!
 * \property QItemModelBarDataMapping::columnCategories
 *
 * Defines the column categories for the mapping.
 */
void QItemModelBarDataMapping::setColumnCategories(const QStringList &categories)
{
    if (dptr()->m_columnCategories != categories) {
        dptr()->m_columnCategories = categories;
        emit mappingChanged();
    }
}

QStringList QItemModelBarDataMapping::columnCategories() const
{
    return dptrc()->m_columnCategories;
}

/*!
 * \property QItemModelBarDataMapping::useModelCategories
 *
 * When set to true, the mapping ignores row and column roles and categories, and uses
 * the rows and columns from the model instead. Defaults to false.
 */
void QItemModelBarDataMapping::setUseModelCategories(bool enable)
{
    if (dptr()->m_useModelCategories != enable) {
        dptr()->m_useModelCategories = enable;
        emit mappingChanged();
    }
}

bool QItemModelBarDataMapping::useModelCategories() const
{
    return dptrc()->m_useModelCategories;
}

/*!
 * \property QItemModelBarDataMapping::autoRowCategories
 *
 * When set to true, the mapping ignores any explicitly set row categories
 * and overwrites them with automatically generated ones whenever the
 * data from model is resolved. Defaults to true.
 */
void QItemModelBarDataMapping::setAutoRowCategories(bool enable)
{
    if (dptr()->m_autoRowCategories != enable) {
        dptr()->m_autoRowCategories = enable;
        emit mappingChanged();
    }
}

bool QItemModelBarDataMapping::autoRowCategories() const
{
    return dptrc()->m_autoRowCategories;
}

/*!
 * \property QItemModelBarDataMapping::autoColumnCategories
 *
 * When set to true, the mapping ignores any explicitly set column categories
 * and overwrites them with automatically generated ones whenever the
 * data from model is resolved. Defaults to true.
 */
void QItemModelBarDataMapping::setAutoColumnCategories(bool enable)
{
    if (dptr()->m_autoColumnCategories != enable) {
        dptr()->m_autoColumnCategories = enable;
        emit mappingChanged();
    }
}

bool QItemModelBarDataMapping::autoColumnCategories() const
{
    return dptrc()->m_autoColumnCategories;
}

/*!
 * Changes \a rowRole, \a columnRole, \a valueRole, \a rowCategories and \a columnCategories to the
 * mapping.
 */
void QItemModelBarDataMapping::remap(const QString &rowRole,
                                     const QString &columnRole,
                                     const QString &valueRole,
                                     const QStringList &rowCategories,
                                     const QStringList &columnCategories)
{
    dptr()->m_rowRole = rowRole;
    dptr()->m_columnRole = columnRole;
    dptr()->m_valueRole = valueRole;
    dptr()->m_rowCategories = rowCategories;
    dptr()->m_columnCategories = columnCategories;

    emit mappingChanged();
}

/*!
 * /return index of the specified \a category in row categories list.
 * If the row categories list is empty, -1 is returned.
 * \note If the automatic row categories generation is in use, this method will
 * not return valid index before the data in the model is resolved for the first time.
 */
int QItemModelBarDataMapping::rowCategoryIndex(const QString &category)
{
    return dptr()->m_rowCategories.indexOf(category);
}

/*!
 * /return index of the specified \a category in column categories list.
 * If the category is not found, -1 is returned.
 * \note If the automatic column categories generation is in use, this method will
 * not return valid index before the data in the model is resolved for the first time.
 */
int QItemModelBarDataMapping::columnCategoryIndex(const QString &category)
{
    return dptr()->m_columnCategories.indexOf(category);
}

/*!
 * \internal
 */
QItemModelBarDataMappingPrivate *QItemModelBarDataMapping::dptr()
{
    return static_cast<QItemModelBarDataMappingPrivate *>(d_ptr.data());
}

/*!
 * \internal
 */
const QItemModelBarDataMappingPrivate *QItemModelBarDataMapping::dptrc() const
{
    return static_cast<const QItemModelBarDataMappingPrivate *>(d_ptr.data());
}

// QItemModelBarDataMappingPrivate

QItemModelBarDataMappingPrivate::QItemModelBarDataMappingPrivate(QItemModelBarDataMapping *q)
    : QAbstractDataMappingPrivate(q, QAbstractDataProxy::DataTypeBar),
      m_useModelCategories(false),
      m_autoRowCategories(true),
      m_autoColumnCategories(true)
{
}

QItemModelBarDataMappingPrivate::~QItemModelBarDataMappingPrivate()
{
}


QT_DATAVISUALIZATION_END_NAMESPACE

