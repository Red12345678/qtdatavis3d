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

#include <QtTest/QtTest>

#include <QtDataVisualization/QLogValue3DAxisFormatter>

using namespace QtDataVisualization;

class tst_axis: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void construct();

    void initialProperties();
    void initializeProperties();
    void invalidProperties();

private:
    QLogValue3DAxisFormatter *m_formatter;
};

void tst_axis::initTestCase()
{
}

void tst_axis::cleanupTestCase()
{
}

void tst_axis::init()
{
    m_formatter = new QLogValue3DAxisFormatter();
}

void tst_axis::cleanup()
{
    delete m_formatter;
}

void tst_axis::construct()
{
    QLogValue3DAxisFormatter *formatter = new QLogValue3DAxisFormatter();
    QVERIFY(formatter);
    delete formatter;
}

void tst_axis::initialProperties()
{
    QVERIFY(m_formatter);

    QCOMPARE(m_formatter->autoSubGrid(), true);
    QCOMPARE(m_formatter->base(), 10.0);
    QCOMPARE(m_formatter->showEdgeLabels(), true);
}

void tst_axis::initializeProperties()
{
    QVERIFY(m_formatter);

    m_formatter->setAutoSubGrid(false);
    m_formatter->setBase(0.1);
    m_formatter->setShowEdgeLabels(false);

    QCOMPARE(m_formatter->autoSubGrid(), false);
    QCOMPARE(m_formatter->base(), 0.1);
    QCOMPARE(m_formatter->showEdgeLabels(), false);
}

void tst_axis::invalidProperties()
{
    m_formatter->setBase(-1.0);
    QCOMPARE(m_formatter->base(), 10.0);

    m_formatter->setBase(1.0);
    QCOMPARE(m_formatter->base(), 10.0);
}

QTEST_MAIN(tst_axis)
#include "tst_axis.moc"
