/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtDataVis3D module.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "q3dbars.h"
#include "q3dbars_p.h"
#include "camerahelper_p.h"
#include "qdataitem_p.h"
#include "qdatarow_p.h"
#include "qdataset_p.h"
#include "shaderhelper_p.h"
#include "objecthelper_p.h"
#include "utils_p.h"

#include <QMatrix4x4>
#include <QOpenGLPaintDevice>
#include <QPainter>
#include <QScreen>
#include <QMouseEvent>

#include <qmath.h>

#include <QDebug>

QTCOMMERCIALDATAVIS3D_BEGIN_NAMESPACE

#define USE_HAX0R_SELECTION // keep this defined until the "real" method works

const float zComp = 10.0f; // Compensation for z position; move all objects to positive z, as shader can't handle negative values correctly
const QVector3D defaultLightPos = QVector3D(0.0f, 3.0f, zComp);

Q3DBars::Q3DBars()
    : d_ptr(new Q3DBarsPrivate(this))
{
}

Q3DBars::~Q3DBars()
{
}

void Q3DBars::initialize()
{
    // Initialize shaders
    if (!d_ptr->m_uniformColor) {
        d_ptr->initShaders(QStringLiteral(":/shaders/vertex")
                           , QStringLiteral(":/shaders/fragmentColorOnY"));
    }
    else {
        d_ptr->initShaders(QStringLiteral(":/shaders/vertex")
                           , QStringLiteral(":/shaders/fragment"));
    }
    d_ptr->initBackgroundShaders(QStringLiteral(":/shaders/vertex")
                                 , QStringLiteral(":/shaders/fragment"));
    d_ptr->initSelectionShader();

#ifndef USE_HAX0R_SELECTION
    // Init the selection buffer
    d_ptr->initSelectionBuffer();
#endif

    // Load default mesh
    d_ptr->loadBarMesh();

    // Load background mesh
    d_ptr->loadBackgroundMesh();

    // Set OpenGL features
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Set initial camera position
    // X must be 0 for rotation to work - we can use "setCameraRotation" for setting it later
    CameraHelper::setDefaultCameraOrientation(QVector3D(0.0f, 0.0f, 6.0f + zComp)
                                              , QVector3D(0.0f, 0.0f, zComp)
                                              , QVector3D(0.0f, 1.0f, 0.0f));

    CameraHelper::setCameraRotation(QPointF(d_ptr->m_horizontalRotation
                                            , d_ptr->m_verticalRotation));

    // Set view port
    glViewport(0, 0, width(), height());

    // Set initialized -flag
    d_ptr->m_isInitialized = true;
}

void Q3DBars::render()
{
    if (!d_ptr->m_isInitialized)
        return;

    if (d_ptr->m_paintDevice) {
        QPainter painter(d_ptr->m_paintDevice);
        painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        //painter.setRenderHint(QPainter::Antialiasing, true);
        render(&painter);
        painter.end();
    }
    else {
        d_ptr->m_paintDevice = getDevice();
    }
}

void Q3DBars::render(QPainter *painter)
{
    painter->beginNativePainting();
    // Set OpenGL features
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    // Do native OpenGL rendering
    drawScene();
    painter->endNativePainting();

    // If a bar is selected, display it's value
    // TODO: Move text printing to a helper class, so that it can be used from other vis types?
    QDataItem *data = d_ptr->m_selectedBar;
    if (data) {
        glDisable(GL_DEPTH_TEST);
        painter->save();
        painter->setCompositionMode(QPainter::CompositionMode_Source);
        // TODO: None of the commented-out stuff works..
        //painter->setBackgroundMode(Qt::OpaqueMode);
        //painter->setBackground(QBrush(d_ptr->m_textBackgroundColor));
        //painter->setBrush(QBrush(d_ptr->m_textBackgroundColor));
        //painter->setPen(d_ptr->m_textBackgroundColor);
        painter->setPen(Qt::black); // TODO: Use black, as nothing works
        QFont bgrFont = QFont(QStringLiteral("Arial"), 18);
        QFont valueFont = QFont(QStringLiteral("Arial"), 12);
        valueFont.setBold(true);
        painter->setFont(bgrFont);
        QFontMetrics valueFM(valueFont);
        QFontMetrics bgrFM(bgrFont);
        int valueStrLen = valueFM.width(data->d_ptr->valueStr());
        int bgrStrLen = 0;
        int bgrHeight = valueFM.height() + 6;
        QString bgrStr = QString();
        do {
            bgrStr.append(QStringLiteral("X"));
            bgrStrLen = bgrFM.width(bgrStr);
        } while (bgrStrLen <= valueStrLen);
        //int bgrLen = valueStrLen + 10;
        //painter->drawRoundedRect(data->position().x() - (bgrLen / 2)
        //                         , data->position().y() - 30
        //                         , bgrLen, 30, 10.0, 10.0);
        // Hack solution, as drawRect doesn't work
        painter->drawText(data->d_ptr->position().x() - (bgrStrLen / 2)
                          , data->d_ptr->position().y() - bgrHeight
                          , bgrStrLen, bgrHeight
                          , Qt::AlignCenter | Qt::AlignVCenter
                          , bgrStr);
        //painter->setPen(d_ptr->m_textColor);
        painter->setPen(Qt::lightGray); // TODO: Use lightGray, as nothing works
        painter->setFont(valueFont);
        painter->drawText(data->d_ptr->position().x() - (valueStrLen / 2)
                          , data->d_ptr->position().y() - bgrHeight
                          , valueStrLen, bgrHeight
                          , Qt::AlignCenter | Qt::AlignVCenter
                          , data->d_ptr->valueStr());
        painter->restore();
    }
}

void Q3DBars::drawScene()
{
    int startBar = 0;
    int stopBar = 0;
    int stepBar = 0;

    int startRow = 0;
    int stopRow = 0;
    int stepRow = 0;

    float backgroundRotation = 0;

    float barPos = 0;
    float rowPos = 0;

    static QVector3D selection = QVector3D(0, 0, 0);

    // Set clear color
    QVector3D clearColor = Utils::vectorFromColor(d_ptr->m_windowColor);
    glClearColor(clearColor.x(), clearColor.y(), clearColor.z(), 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 projectionMatrix;
    projectionMatrix.perspective(45.0f, (float)width() / (float)height(), 0.1f, 100.0f);

    QMatrix4x4 viewMatrix = CameraHelper::calculateViewMatrix(d_ptr->m_mousePos
                                                              , d_ptr->m_zoomLevel
                                                              , width(), height());

    // Calculate drawing order
    //qDebug() << "viewMatrix z" << viewMatrix.row(0).z(); // jos negatiivinen, käännä bar -piirtojärjestys
    //qDebug() << "viewMatrix x" << viewMatrix.row(0).x(); // jos negatiivinen, käännä row -piirtojärjestys
    // TODO: Needs more tuning unless we get depth test working correctly
    // TODO: If depth test gets fixed, the draw order should be reversed for best performance (ie. draw front objects first)
    if (viewMatrix.row(0).x() < 0) {
        startRow = 0;
        stopRow = d_ptr->m_sampleCount.y();
        stepRow = 1;
    }
    else {
        startRow = d_ptr->m_sampleCount.y() - 1;
        stopRow = -1;
        stepRow = -1;
    }
    if (viewMatrix.row(0).z() > 0) {
        startBar = 0;
        stopBar = d_ptr->m_sampleCount.x();
        stepBar = 1;
    }
    else {
        startBar = d_ptr->m_sampleCount.x() - 1;
        stopBar = -1;
        stepBar = -1;
    }

    // calculate background rotation based on view matrix rotation
    if (viewMatrix.row(0).x() >= 0 && viewMatrix.row(0).z() <= 0) {
        backgroundRotation = 270.0f;
    }
    else if (viewMatrix.row(0).x() > 0 && viewMatrix.row(0).z() > 0) {
        backgroundRotation = 180.0f;
    }
    else if (viewMatrix.row(0).x() <= 0 && viewMatrix.row(0).z() >= 0) {
        backgroundRotation = 90.0f;
    }
    else if (viewMatrix.row(0).x() < 0 && viewMatrix.row(0).z() < 0) {
        backgroundRotation = 0.0f;
    }

    // Get light position (rotate light with camera, a bit above it (as set in defaultLightPos))
    QVector3D lightPos = CameraHelper::calculateLightPosition(defaultLightPos);
    //lightPos = QVector3D(0.0f, 4.0f, zComp); // center of bars, 4.0f above - for testing

    // Bind selection shader
    d_ptr->m_selectionShader->bind();

    // Draw bars to selection buffer
    glDisable(GL_DITHER); // disable dithering, it may affect colors if enabled
    for (int row = startRow; row != stopRow; row += stepRow) {
        for (int bar = startBar; bar != stopBar; bar += stepBar) {
            if (d_ptr->m_dataSet.at(row).size() < (bar + 1))
                continue;
            QDataItem *item = d_ptr->m_dataSet.at(row).at(bar);
            float barHeight = item->d_ptr->value() / d_ptr->m_heightNormalizer;
            QMatrix4x4 modelMatrix;
            QMatrix4x4 MVPMatrix;
            barPos = (bar + 1) * (d_ptr->m_barSpacing.x());
            rowPos = (row + 1) * (d_ptr->m_barSpacing.y());
            modelMatrix.translate((d_ptr->m_rowWidth - barPos) / d_ptr->m_scaleFactorX
                                  , barHeight - 1.0f
                                  , (d_ptr->m_columnDepth - rowPos) / d_ptr->m_scaleFactorZ + zComp);
            modelMatrix.scale(QVector3D(d_ptr->m_scaleX, barHeight, d_ptr->m_scaleZ));

            MVPMatrix = projectionMatrix * viewMatrix * modelMatrix;

            // add +2 to avoid black
            QVector3D barColor = QVector3D((float)(row + 2) / (float)(d_ptr->m_sampleCount.y() + 2)
                                           , (float)(bar + 2) / (float)(d_ptr->m_sampleCount.x() + 2)
                                           , 0.0f);

            d_ptr->m_selectionShader->setUniformValue(d_ptr->m_selectionShader->MVP()
                                                      , MVPMatrix);
            d_ptr->m_selectionShader->setUniformValue(d_ptr->m_selectionShader->color()
                                                      , barColor);

#ifdef USE_HAX0R_SELECTION
            // 1st attribute buffer : vertices
            glEnableVertexAttribArray(d_ptr->m_selectionShader->posAtt());
            glBindBuffer(GL_ARRAY_BUFFER, d_ptr->m_barObj->vertexBuf());
            glVertexAttribPointer(d_ptr->m_selectionShader->posAtt()
                                  , 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            // Index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d_ptr->m_barObj->elementBuf());

            // Draw the triangles
            glDrawElements(GL_TRIANGLES, d_ptr->m_barObj->indexCount()
                           , GL_UNSIGNED_SHORT, (void*)0);

            // Free buffers
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            glDisableVertexAttribArray(d_ptr->m_selectionShader->posAtt());
#else // TODO: fix this - doesn't work yet
            glBindFramebuffer(GL_FRAMEBUFFER, d_ptr->m_framebufferSelection);
            //glReadBuffer(GL_COLOR_ATTACHMENT0);

            // 1st attribute buffer : vertices
            glEnableVertexAttribArray(d_ptr->m_selectionShader->posAtt());
            glBindBuffer(GL_ARRAY_BUFFER, d_ptr->m_barObj->vertexBuf());
            glVertexAttribPointer(d_ptr->m_selectionShader->posAtt()
                                  , 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            // Index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d_ptr->m_barObj->elementBuf());

            // Draw the triangles
            GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
            glDrawElements(GL_TRIANGLES, d_ptr->m_barObj->indexCount()
                           , GL_UNSIGNED_SHORT, DrawBuffers);

            glDisableVertexAttribArray(d_ptr->m_selectionShader->posAtt());

            // Free buffers
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            //glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
        }
    }
    glEnable(GL_DITHER);

    // Read color under cursor
    // TODO: Move into a separate function?
    if (d_ptr->m_mousePressed) {
#ifndef USE_HAX0R_SELECTION
        glBindFramebuffer(GL_FRAMEBUFFER, d_ptr->m_framebufferSelection);
#endif
        // This is the only one that works with ANGLE (ES 2.0)
        // Item count will be limited to 256*256*256
        GLubyte pixel[4];
        glReadPixels(d_ptr->m_mousePos.x(), height() - d_ptr->m_mousePos.y(), 1, 1,
                     GL_RGBA, GL_UNSIGNED_BYTE, (void *)pixel);

        // These work with desktop OpenGL
        // They offer a lot higher possible object count and a possibility to use object id's
        //GLuint pixel2[3];
        //glReadPixels(d_ptr->m_mousePos.x(), height() - d_ptr->m_mousePos.y(), 1, 1,
        //             GL_RGB, GL_UNSIGNED_INT, (void *)pixel2);

        //GLfloat pixel3[3];
        //glReadPixels(d_ptr->m_mousePos.x(), height() - d_ptr->m_mousePos.y(), 1, 1,
        //             GL_RGB, GL_FLOAT, (void *)pixel3);

        //qDebug() << "rgba" << pixel[0] << pixel[1] << pixel[2];// << pixel[3];
        //qDebug() << "rgba2" << pixel2[0] << pixel2[1] << pixel2[2];
        //qDebug() << "rgba3" << pixel3[0] << pixel3[1] << pixel3[2];
        selection = QVector3D(pixel[0], pixel[1], pixel[2]);
        //qDebug() << selection;
#ifndef USE_HAX0R_SELECTION
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
    }

    // Release selection shader
    d_ptr->m_selectionShader->release();

    // Clear after selection
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind background shader
    d_ptr->m_backgroundShader->bind();

    // Draw background
    if (d_ptr->m_backgroundObj) {
        QMatrix4x4 modelMatrix;
        QMatrix4x4 MVPMatrix;
        if (zComp != 0)
            modelMatrix.translate(0.0f, 0.0f, zComp);
        modelMatrix.scale(QVector3D(d_ptr->m_rowWidth * d_ptr->m_sceneScale
                                    , 1.0f
                                    , d_ptr->m_columnDepth * d_ptr->m_sceneScale));
        modelMatrix.rotate(backgroundRotation, 0.0f, 1.0f, 0.0f);

        MVPMatrix = projectionMatrix * viewMatrix * modelMatrix;

        QVector3D backgroundColor = Utils::vectorFromColor(d_ptr->m_backgroundColor);

        d_ptr->m_backgroundShader->setUniformValue(d_ptr->m_backgroundShader->lightP()
                                                   , lightPos);
        d_ptr->m_backgroundShader->setUniformValue(d_ptr->m_backgroundShader->view()
                                                   , viewMatrix);
        d_ptr->m_backgroundShader->setUniformValue(d_ptr->m_backgroundShader->model()
                                                   , modelMatrix);
        d_ptr->m_backgroundShader->setUniformValue(d_ptr->m_backgroundShader->nModel()
                                                   , modelMatrix.inverted().transposed());
        d_ptr->m_backgroundShader->setUniformValue(d_ptr->m_backgroundShader->MVP()
                                                   , MVPMatrix);
        d_ptr->m_backgroundShader->setUniformValue(d_ptr->m_backgroundShader->color()
                                                   , backgroundColor);
        d_ptr->m_backgroundShader->setUniformValue(d_ptr->m_backgroundShader->lightS()
                                                   , d_ptr->m_lightStrength);
        d_ptr->m_backgroundShader->setUniformValue(d_ptr->m_backgroundShader->ambientS()
                                                   , d_ptr->m_ambientStrength);

        // 1st attribute buffer : vertices
        glEnableVertexAttribArray(d_ptr->m_backgroundShader->posAtt());
        glBindBuffer(GL_ARRAY_BUFFER, d_ptr->m_backgroundObj->vertexBuf());
        glVertexAttribPointer(d_ptr->m_backgroundShader->posAtt()
                              , 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // 2nd attribute buffer : normals
        glEnableVertexAttribArray(d_ptr->m_backgroundShader->normalAtt());
        glBindBuffer(GL_ARRAY_BUFFER, d_ptr->m_backgroundObj->normalBuf());
        glVertexAttribPointer(d_ptr->m_backgroundShader->normalAtt()
                              , 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // 3rd attribute buffer : UVs
        //glEnableVertexAttribArray(d_ptr->m_backgroundShader->uvAtt());
        //glBindBuffer(GL_ARRAY_BUFFER, d_ptr->m_backgroundObj->uvBuf());
        //glVertexAttribPointer(d_ptr->m_backgroundShader->uvAtt()
        //                      , 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d_ptr->m_backgroundObj->elementBuf());

        // Draw the triangles
        glDrawElements(GL_TRIANGLES, d_ptr->m_backgroundObj->indexCount()
                       , GL_UNSIGNED_SHORT, (void*)0);

        // Free buffers
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        //glDisableVertexAttribArray(d_ptr->m_backgroundShader->uvAtt());
        glDisableVertexAttribArray(d_ptr->m_backgroundShader->normalAtt());
        glDisableVertexAttribArray(d_ptr->m_backgroundShader->posAtt());
    }

    // Release background shader
    d_ptr->m_backgroundShader->release();

    // Bind bar shader
    d_ptr->m_barShader->bind();

    // Draw bars
    //int barIndex = 1; // TODO: Remove when done debugging
    bool barSelectionFound = false;
    for (int row = startRow; row != stopRow; row += stepRow) {
        for (int bar = startBar; bar != stopBar; bar += stepBar) {
            if (d_ptr->m_dataSet.at(row).size() < (bar + 1))
                continue;
            QDataItem *item = d_ptr->m_dataSet.at(row).at(bar);
            float barHeight = item->d_ptr->value() / d_ptr->m_heightNormalizer;
            if (barHeight == 0)
                continue;
            QMatrix4x4 modelMatrix;
            QMatrix4x4 MVPMatrix;
            barPos = (bar + 1) * (d_ptr->m_barSpacing.x());
            //qDebug() << "x" << d_ptr->m_rowWidth << "-" << barPos << "=" << d_ptr->m_rowWidth - barPos;
            rowPos = (row + 1) * (d_ptr->m_barSpacing.y());
            //qDebug() << "z" << rowPos << "-" << d_ptr->m_columnDepth << "=" << rowPos - d_ptr->m_columnDepth;
            modelMatrix.translate((d_ptr->m_rowWidth - barPos) / d_ptr->m_scaleFactorX
                                  , barHeight - 1.0f
                                  , (d_ptr->m_columnDepth - rowPos) / d_ptr->m_scaleFactorZ + zComp);
            modelMatrix.scale(QVector3D(d_ptr->m_scaleX, barHeight, d_ptr->m_scaleZ));

            MVPMatrix = projectionMatrix * viewMatrix * modelMatrix;

            QVector3D baseColor = Utils::vectorFromColor(d_ptr->m_baseColor);
            QVector3D heightColor = Utils::vectorFromColor(d_ptr->m_heightColor) * barHeight;
            QVector3D depthColor = Utils::vectorFromColor(d_ptr->m_depthColor)
                    * (float(row) / float(d_ptr->m_sampleCount.y()));

            QVector3D barColor = baseColor + heightColor + depthColor;

            float lightStrength = d_ptr->m_lightStrength;
            if (d_ptr->m_selectionMode > None) {
                Q3DBarsPrivate::SelectionType selectionType = d_ptr->isSelected(row, bar
                                                                                , selection);
                switch (selectionType) {
                case Q3DBarsPrivate::Bar:
                {
                    // highlight bar by inverting the color of the bar
                    //barColor = QVector3D(1.0f, 1.0f, 1.0f) - barColor;
                    barColor = Utils::vectorFromColor(d_ptr->m_highlightBarColor);
                    lightStrength = d_ptr->m_highlightLightStrength;
                    //if (d_ptr->m_mousePressed) {
                    //    qDebug() << "selected object:" << barIndex << "( row:" << row + 1 << ", column:" << bar + 1 << ")";
                    //    qDebug() /*<< barIndex*/ << "object position:" << modelMatrix.column(3).toVector3D();
                    //}
                    // Insert data to QDataItem. We have no ownership, don't delete the previous one
                    d_ptr->m_selectedBar = item;
                    d_ptr->m_selectedBar->d_ptr->setPosition(d_ptr->m_mousePos);
                    barSelectionFound = true;
                    break;
                }
                case Q3DBarsPrivate::Row:
                {
                    // Current bar is on the same row as the selected bar
                    barColor = Utils::vectorFromColor(d_ptr->m_highlightRowColor);
                    lightStrength = d_ptr->m_highlightLightStrength;
                    break;
                }
                case Q3DBarsPrivate::Column:
                {
                    // Current bar is on the same column as the selected bar
                    barColor = Utils::vectorFromColor(d_ptr->m_highlightColumnColor);
                    lightStrength = d_ptr->m_highlightLightStrength;
                    break;
                }
                case Q3DBarsPrivate::None:
                {
                    // Current bar is not selected, nor on a row or column
                    // do nothing
                    break;
                }
                }
            }

            //if (d_ptr->m_mousePressed) {
                //qDebug() << "baseColor:" << baseColor;
                //qDebug() << "heightColor:" << heightColor;
                //qDebug() << "depthColor:" << depthColor;
                //qDebug() << "barColor:" << barColor;
                //qDebug() << barIndex << "object position:" << modelMatrix.column(3).toVector3D();
            //}
            //barIndex++; // TODO: Remove when done debugging

            d_ptr->m_barShader->setUniformValue(d_ptr->m_barShader->lightP(), lightPos);
            d_ptr->m_barShader->setUniformValue(d_ptr->m_barShader->view(), viewMatrix);
            d_ptr->m_barShader->setUniformValue(d_ptr->m_barShader->model(), modelMatrix);
            d_ptr->m_barShader->setUniformValue(d_ptr->m_barShader->nModel()
                                                , modelMatrix.inverted().transposed());
            d_ptr->m_barShader->setUniformValue(d_ptr->m_barShader->MVP(), MVPMatrix);
            d_ptr->m_barShader->setUniformValue(d_ptr->m_barShader->color(), barColor);
            d_ptr->m_barShader->setUniformValue(d_ptr->m_barShader->lightS(), lightStrength);
            d_ptr->m_barShader->setUniformValue(d_ptr->m_barShader->ambientS()
                                                , d_ptr->m_ambientStrength);
            //qDebug() << "height:" << barHeight;

            // 1st attribute buffer : vertices
            glEnableVertexAttribArray(d_ptr->m_barShader->posAtt());
            glBindBuffer(GL_ARRAY_BUFFER, d_ptr->m_barObj->vertexBuf());
            glVertexAttribPointer(d_ptr->m_barShader->posAtt()
                                  , 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            // 2nd attribute buffer : normals
            glEnableVertexAttribArray(d_ptr->m_barShader->normalAtt());
            glBindBuffer(GL_ARRAY_BUFFER, d_ptr->m_barObj->normalBuf());
            glVertexAttribPointer(d_ptr->m_barShader->normalAtt()
                                  , 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            // 3rd attribute buffer : UVs
            //glEnableVertexAttribArray(d_ptr->m_barShader->m_uvAtt());
            //glBindBuffer(GL_ARRAY_BUFFER, d_ptr->m_barObj->uvBuf());
            //glVertexAttribPointer(d_ptr->m_barShader->m_uvAtt()
            //                      , 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

            // Index buffer
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d_ptr->m_barObj->elementBuf());

            // Draw the triangles
            glDrawElements(GL_TRIANGLES, d_ptr->m_barObj->indexCount()
                           , GL_UNSIGNED_SHORT, (void*)0);

            // Free buffers
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            //glDisableVertexAttribArray(d_ptr->m_barShader->m_uvAtt());
            glDisableVertexAttribArray(d_ptr->m_barShader->normalAtt());
            glDisableVertexAttribArray(d_ptr->m_barShader->posAtt());
        }
    }
    if (!barSelectionFound) {
        // We have no ownership, don't delete. Just NULL the pointer.
        d_ptr->m_selectedBar = NULL;
    }

    // Release bar shader
    d_ptr->m_barShader->release();
}

void Q3DBars::mousePressEvent(QMouseEvent *event)
{
    // TODO: for testing shaders
    static int shaderNo = 1;
    //qDebug() << "mouse button pressed" << event->button();
    if (Qt::LeftButton == event->button()) {
        d_ptr->m_mousePressed = true;
        // update mouse positions to prevent jumping when releasing or repressing a button
        d_ptr->m_mousePos = event->pos();
    }
    else if (Qt::RightButton == event->button()) {
        // reset rotations
        d_ptr->m_mousePos = QPoint(0, 0);
    }
    else if (Qt::MiddleButton == event->button()) {
        // TODO: testing shaders
        if (++shaderNo > 3)
            shaderNo = 1;
        switch (shaderNo) {
        case 1:
            d_ptr->initShaders(QStringLiteral(":/shaders/vertex")
                               , QStringLiteral(":/shaders/fragment"));
            break;
        case 2:
            d_ptr->initShaders(QStringLiteral(":/shaders/vertex")
                               , QStringLiteral(":/shaders/fragmentColorOnY"));
            break;
        case 3:
            d_ptr->initShaders(QStringLiteral(":/shaders/vertex")
                               , QStringLiteral(":/shaders/fragmentAmbient"));
            break;
        }
    }
    CameraHelper::updateMousePos(d_ptr->m_mousePos);
}

void Q3DBars::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << "mouse button released" << event->button();
    d_ptr->m_mousePressed = false;
    // update mouse positions to prevent jumping when releasing or repressing a button
    d_ptr->m_mousePos = event->pos();
    CameraHelper::updateMousePos(event->pos());
}

void Q3DBars::mouseMoveEvent(QMouseEvent *event)
{
    if (d_ptr->m_mousePressed) {
        //qDebug() << "mouse moved while pressed" << event->pos();
        d_ptr->m_mousePos = event->pos();
    }
#if 0
    // TODO: Testi - laske kursorin sijainti scenessä
    QPointF mouse3D((2.0f * event->pos().x() - width()) / height()
                    , 1.0f - (2.0f * event->pos().y()) / height());
    //qDebug() << "mouse position in scene" << mouse3D;

    // TODO: Testi laske focal point
    float focalPoint = tan(45.0f / 2.0f);

    // TODO: Testi - laske viewmatriisin kerroin
    QVector3D worldRay = QVector3D(0.0f, 0.0f, 0.0f)
            - QVector3D(mouse3D.x(), mouse3D.y(), -focalPoint);
    //qDebug() << "worldRay" << worldRay;
    // multiply viewmatrix with this to get something?
#endif
}

void Q3DBars::wheelEvent(QWheelEvent *event)
{
    if (d_ptr->m_zoomLevel > 100) {
        d_ptr->m_zoomLevel += event->angleDelta().y() / 12;
    }
    else if (d_ptr->m_zoomLevel > 50) {
        d_ptr->m_zoomLevel += event->angleDelta().y() / 60;
    }
    else {
        d_ptr->m_zoomLevel += event->angleDelta().y() / 120;
    }
    if (d_ptr->m_zoomLevel > 500) {
        d_ptr->m_zoomLevel = 500;
    }
    else if (d_ptr->m_zoomLevel < 10) {
        d_ptr->m_zoomLevel = 10;
    }
}

void Q3DBars::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    // Set view port
    glViewport(0, 0, width(), height());

    // If orientation changes, we need to scale again
    // TODO: Handle it
}

void Q3DBars::setBarSpecs(QPointF thickness, QPointF spacing, bool relative)
{
    d_ptr->m_barThickness = thickness;
    if (relative) {
        d_ptr->m_barSpacing.setX((thickness.x() * 2) * (spacing.x() + 1.0f));
        d_ptr->m_barSpacing.setY((thickness.y() * 2) * (spacing.y() + 1.0f));
    }
    else {
        d_ptr->m_barSpacing = thickness * 2 + spacing * 2;
    }
    // Calculate here and at setting sample space
    d_ptr->calculateSceneScalingFactors();
}

void Q3DBars::setBarType(BarStyle style, bool smooth)
{
    if (style == Bars) {
        if (smooth) {
            d_ptr->m_objFile = QStringLiteral(":/defaultMeshes/barSmooth");
        }
        else {
            d_ptr->m_objFile = QStringLiteral(":/defaultMeshes/bar");
        }
    }
    else if (style == Pyramids) {
        if (smooth) {
            d_ptr->m_objFile = QStringLiteral(":/defaultMeshes/pyramidSmooth");
        }
        else {
            d_ptr->m_objFile = QStringLiteral(":/defaultMeshes/pyramid");
        }
    }
    else if (style == Cones) {
        if (smooth) {
            d_ptr->m_objFile = QStringLiteral(":/defaultMeshes/coneSmooth");
        }
        else {
            d_ptr->m_objFile = QStringLiteral(":/defaultMeshes/cone");
        }
    }
    else if (style == Cylinders) {
        if (smooth) {
            d_ptr->m_objFile = QStringLiteral(":/defaultMeshes/cylinderSmooth");
        }
        else {
            d_ptr->m_objFile = QStringLiteral(":/defaultMeshes/cylinder");
        }
    }
    if (d_ptr->m_isInitialized) {
        // Reload mesh data
        d_ptr->loadBarMesh();
    }
}

void Q3DBars::setMeshFileName(const QString &objFileName)
{
    d_ptr->m_objFile = objFileName;
}

void Q3DBars::setupSampleSpace(QPoint sampleCount, const QString &labelRow
        , const QString &labelColumn, const QString &labelHeight)
{
    d_ptr->m_sampleCount = sampleCount;
    // Initialize data set
    QVector<QDataItem*> row;
    for (int rows = 0; rows < sampleCount.y(); rows++) {
        for (int columns = 0; columns < sampleCount.x(); columns ++) {
            row.append(new QDataItem());
        }
        d_ptr->m_dataSet.append(row);
        row.clear();
    }
    // TODO: Invent "idiotproof" max scene size formula..
    // This seems to work ok if spacing is not negative
    d_ptr->m_maxSceneSize = 2 * qSqrt(sampleCount.x() * sampleCount.y());
    //qDebug() << "maxSceneSize" << d_ptr->m_maxSceneSize;
    // Calculate here and at setting bar specs
    d_ptr->calculateSceneScalingFactors();
}

void Q3DBars::setCameraPreset(CameraPreset preset)
{
    // TODO: This should be moved to CameraHelper to be directly usable by other vis types - just call directly a function there?
    // TODO: Move these enums to namespace level?
    switch (preset) {
    case PresetFrontLow: {
        qDebug("PresetFrontLow");
        setCameraPosition(0.0f, 0.0f);
        break;
    }
    case PresetFront: {
        qDebug("PresetFront");
        setCameraPosition(0.0f, 22.5f);
        break;
    }
    case PresetFrontHigh: {
        qDebug("PresetFrontHigh");
        setCameraPosition(0.0f, 45.0f);
        break;
    }
    case PresetLeftLow: {
        qDebug("PresetLeftLow");
        setCameraPosition(90.0f, 0.0f);
        break;
    }
    case PresetLeft: {
        qDebug("PresetLeft");
        setCameraPosition(90.0f, 22.5f);
        break;
    }
    case PresetLeftHigh: {
        qDebug("PresetLeftHigh");
        setCameraPosition(90.0f, 45.0f);
        break;
    }
    case PresetRightLow: {
        qDebug("PresetRightLow");
        setCameraPosition(-90.0f, 0.0f);
        break;
    }
    case PresetRight: {
        qDebug("PresetRight");
        setCameraPosition(-90.0f, 22.5f);
        break;
    }
    case PresetRightHigh: {
        qDebug("PresetRightHigh");
        setCameraPosition(-90.0f, 45.0f);
        break;
    }
    case PresetBehindLow: {
        qDebug("PresetBehindLow");
        setCameraPosition(180.0f, 0.0f);
        break;
    }
    case PresetBehind: {
        qDebug("PresetBehind");
        setCameraPosition(180.0f, 22.5f);
        break;
    }
    case PresetBehindHigh: {
        qDebug("PresetBehindHigh");
        setCameraPosition(180.0f, 45.0f);
        break;
    }
    case PresetIsometricLeft: {
        qDebug("PresetIsometricLeft");
        setCameraPosition(45.0f, 22.5f);
        break;
    }
    case PresetIsometricLeftHigh: {
        qDebug("PresetIsometricLeftHigh");
        setCameraPosition(45.0f, 45.0f);
        break;
    }
    case PresetIsometricRight: {
        qDebug("PresetIsometricRight");
        setCameraPosition(-45.0f, 22.5f);
        break;
    }
    case PresetIsometricRightHigh: {
        qDebug("PresetIsometricRightHigh");
        setCameraPosition(-45.0f, 45.0f);
        break;
    }
    case PresetDirectlyAbove: {
        qDebug("PresetDirectlyAbove");
        setCameraPosition(0.0f, 90.0f);
        break;
    }
    case PresetDirectlyAboveCW45: {
        qDebug("PresetDirectlyAboveCW45");
        setCameraPosition(-45.0f, 90.0f);
        break;
    }
    case PresetDirectlyAboveCCW45: {
        qDebug("PresetDirectlyAboveCCW45");
        setCameraPosition(45.0f, 90.0f);
        break;
    }
    default:
        break;
    }
}

void Q3DBars::setCameraPosition(float horizontal, float vertical, int distance)
{
    d_ptr->m_horizontalRotation = qBound(-180.0f, horizontal, 180.0f);
    d_ptr->m_verticalRotation = qBound(0.0f, vertical, 90.0f);
    d_ptr->m_zoomLevel = qBound(10, distance, 500);
    CameraHelper::setCameraRotation(QPointF(d_ptr->m_horizontalRotation
                                            , d_ptr->m_verticalRotation));
    qDebug() << "camera rotation set to" << d_ptr->m_horizontalRotation << d_ptr->m_verticalRotation;
}

void Q3DBars::setTheme(ColorTheme theme)
{
    // TODO: Get themes from a theme class? Move enum to namespace level?
    // TODO: Hardcoded here at first..
    switch (theme) {
    case ThemeSystem: {
//        d_ptr->m_baseColor = QColor();
//        d_ptr->m_heightColor = QColor();
//        d_ptr->m_depthColor = QColor();
//        d_ptr->m_backgroundColor = QColor();
//        d_ptr->m_windowColor = QColor();
//        d_ptr->m_textColor = QColor();
//        d_ptr->m_textBackgroundColor = QColor();
//        d_ptr->m_highlightBarColor = QColor();
//        d_ptr->m_highlightRowColor = QColor();
//        d_ptr->m_highlightColumnColor = QColor();
//        d_ptr->m_lightStrength = QColor();
//        d_ptr->m_ambientStrength = QColor();
//        d_ptr->m_highlightLightStrength = QColor();
//        d_ptr->m_uniformColor = QColor();
        break;
    }
    case ThemeBlueCerulean: {
        d_ptr->m_baseColor = QColor(QRgb(0xc7e85b));
        d_ptr->m_heightColor = QColor(QRgb(0xee7392));
        d_ptr->m_depthColor = QColor(Qt::black);
        d_ptr->m_backgroundColor = QColor(QRgb(0x056189));
        d_ptr->m_windowColor = QColor(QRgb(0x101a31));
        d_ptr->m_textColor = QColor(QRgb(0xffffff));
        d_ptr->m_textBackgroundColor = QColor(QRgb(0xd6d6d6));
        d_ptr->m_highlightBarColor = QColor(Qt::blue);
        d_ptr->m_highlightRowColor = QColor(Qt::darkBlue);
        d_ptr->m_highlightColumnColor = QColor(Qt::darkBlue);
        d_ptr->m_lightStrength = 5.0f;
        d_ptr->m_ambientStrength = 0.2f;
        d_ptr->m_highlightLightStrength = 10.0f;
        d_ptr->m_uniformColor = true;
        qDebug("ThemeBlueCerulean");
        break;
    }
    case ThemeBlueIcy: {
        d_ptr->m_baseColor = QRgb(0x3daeda);
        d_ptr->m_heightColor = QRgb(0x2fa3b4);
        d_ptr->m_depthColor = QColor(Qt::lightGray);
        d_ptr->m_backgroundColor = QColor(QRgb(0xffffff));
        d_ptr->m_windowColor = QColor(QRgb(0xffffff));
        d_ptr->m_textColor = QColor(QRgb(0x404044));
        d_ptr->m_textBackgroundColor = QColor(QRgb(0xd6d6d6));
        d_ptr->m_highlightBarColor = QColor(Qt::white);
        d_ptr->m_highlightRowColor = QColor(Qt::lightGray);
        d_ptr->m_highlightColumnColor = QColor(Qt::lightGray);
        d_ptr->m_lightStrength = 5.0f;
        d_ptr->m_ambientStrength = 0.4f;
        d_ptr->m_highlightLightStrength = 8.0f;
        d_ptr->m_uniformColor = true;
        qDebug("ThemeBlueIcy");
        break;
    }
    case ThemeBlueNcs: {
        d_ptr->m_baseColor = QColor(QRgb(0x1db0da));
        d_ptr->m_heightColor = QColor(QRgb(0x398ca3));
        d_ptr->m_depthColor = QColor(Qt::lightGray);
        d_ptr->m_backgroundColor = QColor(QRgb(0xffffff));
        d_ptr->m_windowColor = QColor(QRgb(0xffffff));
        d_ptr->m_textColor = QColor(QRgb(0x404044));
        d_ptr->m_textBackgroundColor = QColor(QRgb(0xd6d6d6));
        d_ptr->m_highlightBarColor = QColor(Qt::lightGray);
        d_ptr->m_highlightRowColor = QColor(Qt::gray);
        d_ptr->m_highlightColumnColor = QColor(Qt::gray);
        d_ptr->m_lightStrength = 4.0f;
        d_ptr->m_ambientStrength = 0.2f;
        d_ptr->m_highlightLightStrength = 6.0f;
        d_ptr->m_uniformColor = true;
        qDebug("ThemeBlueNcs");
        break;
    }
    case ThemeBrownSand: {
        d_ptr->m_baseColor = QColor(QRgb(0xb39b72));
        d_ptr->m_heightColor = QColor(QRgb(0x494345));
        d_ptr->m_depthColor = QColor(Qt::darkYellow);
        d_ptr->m_backgroundColor = QColor(QRgb(0xf3ece0));
        d_ptr->m_windowColor = QColor(QRgb(0xf3ece0));
        d_ptr->m_textColor = QColor(QRgb(0x404044));
        d_ptr->m_textBackgroundColor = QColor(QRgb(0xb5b0a7));
        d_ptr->m_highlightBarColor = QColor(Qt::yellow);
        d_ptr->m_highlightRowColor = QColor(Qt::darkYellow);
        d_ptr->m_highlightColumnColor = QColor(Qt::darkYellow);
        d_ptr->m_lightStrength = 6.0f;
        d_ptr->m_ambientStrength = 0.3f;
        d_ptr->m_highlightLightStrength = 8.0f;
        d_ptr->m_uniformColor = false;
        qDebug("ThemeBrownSand");
        break;
    }
    case ThemeDark: {
        d_ptr->m_baseColor = QColor(QRgb(0x38ad6b));
        d_ptr->m_heightColor = QColor(QRgb(0xbf593e));
        d_ptr->m_depthColor = QColor(Qt::black);
        d_ptr->m_backgroundColor = QColor(QRgb(0x2e303a));
        d_ptr->m_windowColor = QColor(QRgb(0x121218));
        d_ptr->m_textColor = QColor(QRgb(0xffffff));
        d_ptr->m_textBackgroundColor = QColor(QRgb(0x86878c));
        d_ptr->m_highlightBarColor = QColor(Qt::gray);
        d_ptr->m_highlightRowColor = QColor(Qt::darkGray);
        d_ptr->m_highlightColumnColor = QColor(Qt::darkGray);
        d_ptr->m_lightStrength = 6.0f;
        d_ptr->m_ambientStrength = 0.2f;
        d_ptr->m_highlightLightStrength = 8.0f;
        d_ptr->m_uniformColor = false;
        qDebug("ThemeDark");
        break;
    }
    case ThemeHighContrast: {
        d_ptr->m_baseColor = QColor(QRgb(0x202020));
        d_ptr->m_heightColor = QColor(QRgb(0xff4a41));
        d_ptr->m_depthColor = QColor(Qt::red);
        d_ptr->m_backgroundColor = QColor(QRgb(0xffffff));
        d_ptr->m_windowColor = QColor(QRgb(0xffffff));
        d_ptr->m_textColor = QColor(QRgb(0x181818));
        d_ptr->m_textBackgroundColor = QColor(QRgb(0x8c8c8c));
        d_ptr->m_highlightBarColor = QColor(Qt::black);
        d_ptr->m_highlightRowColor = QColor(Qt::white);
        d_ptr->m_highlightColumnColor = QColor(Qt::white);
        d_ptr->m_lightStrength = 5.0f;
        d_ptr->m_ambientStrength = 1.0f;
        d_ptr->m_highlightLightStrength = 10.0f;
        d_ptr->m_uniformColor = false;
        qDebug("ThemeHighContrast");
        break;
    }
    case ThemeLight: {
        d_ptr->m_baseColor = QColor(QRgb(0x209fdf));
        d_ptr->m_heightColor = QColor(QRgb(0xbf593e));
        d_ptr->m_depthColor = QColor(Qt::lightGray);
        d_ptr->m_backgroundColor = QColor(QRgb(0xffffff));
        d_ptr->m_windowColor = QColor(QRgb(0xffffff));
        d_ptr->m_textColor = QColor(QRgb(0x404044));
        d_ptr->m_textBackgroundColor = QColor(QRgb(0xd6d6d6));
        d_ptr->m_highlightBarColor = QColor(Qt::white);
        d_ptr->m_highlightRowColor = QColor(Qt::lightGray);
        d_ptr->m_highlightColumnColor = QColor(Qt::lightGray);
        d_ptr->m_lightStrength = 3.0f;
        d_ptr->m_ambientStrength = 0.5f;
        d_ptr->m_highlightLightStrength = 6.0f;
        d_ptr->m_uniformColor = true;
        qDebug("ThemeLight");
        break;
    }
    default:
        break;
    }
    // Re-initialize shaders
    if (!d_ptr->m_uniformColor) {
        d_ptr->initShaders(QStringLiteral(":/shaders/vertex")
                           , QStringLiteral(":/shaders/fragmentColorOnY"));
    }
    else {
        d_ptr->initShaders(QStringLiteral(":/shaders/vertex")
                           , QStringLiteral(":/shaders/fragment"));
    }
}

void Q3DBars::setBarColor(QColor baseColor, QColor heightColor, QColor depthColor, bool uniform)
{
    d_ptr->m_baseColor = baseColor;
    d_ptr->m_heightColor = heightColor;
    d_ptr->m_depthColor = depthColor;
    //qDebug() << "colors:" << d_ptr->m_baseColor << d_ptr->m_heightColor << d_ptr->m_depthColor;
    if (d_ptr->m_uniformColor != uniform) {
        // Re-initialize shaders
        if (!d_ptr->m_uniformColor) {
            d_ptr->initShaders(QStringLiteral(":/shaders/vertex")
                               , QStringLiteral(":/shaders/fragmentColorOnY"));
        }
        else {
            d_ptr->initShaders(QStringLiteral(":/shaders/vertex")
                               , QStringLiteral(":/shaders/fragment"));
        }
    }
    d_ptr->m_uniformColor = uniform;
}

void Q3DBars::setSelectionMode(SelectionMode mode)
{
    d_ptr->m_selectionMode = mode;
}

void Q3DBars::setWindowTitle(const QString &title)
{
    setTitle(title);
}

void Q3DBars::addDataRow(const QVector<float> &dataRow, const QString &labelRow
                         , const QVector<QString> &labelsColumn)
{
    QVector<float> row = dataRow;
    // Check that the input data fits into sample space, and resize if it doesn't
    if (row.size() > d_ptr->m_sampleCount.x()) {
        row.resize(d_ptr->m_sampleCount.x());
        qWarning("Data set too large for sample space");
    }
    // Convert row of floats into sample data
    QVector<QDataItem*> sampleRow;
    for (int i = 0; i < row.size(); i++) {
        sampleRow.append(new QDataItem(row.at(i)));
    }
    d_ptr->findHighestValue(sampleRow);
    // The vector contains data (=height) for each bar, a row at a time
    // With each new row, the previous data row must be moved back
    // ie. we need as many vectors as we have rows in the sample space
    d_ptr->m_dataSet.prepend(sampleRow);
    // if the added data pushed us over sample space, remove the oldest data set
    if (d_ptr->m_dataSet.size() > d_ptr->m_sampleCount.y())
        d_ptr->resizeDataSet();
}

void Q3DBars::addDataRow(const QVector<QDataItem*> &dataRow, const QString &labelRow
                         , const QVector<QString> &labelsColumn)
{
    QVector<QDataItem*> row = dataRow;
    // Check that the input data fits into sample space, and resize if it doesn't
    if (row.size() > d_ptr->m_sampleCount.x()) {
        d_ptr->resizeDataRow(&row);
        qWarning("Data set too large for sample space");
    }
    d_ptr->findHighestValue(row);
    // With each new row, the previous data row must be moved back
    // ie. we need as many vectors as we have rows in the sample space
    d_ptr->m_dataSet.prepend(row);
    // if the added data pushed us over sample space, remove the oldest data set
    if (d_ptr->m_dataSet.size() > d_ptr->m_sampleCount.y())
        d_ptr->resizeDataSet();
}

void Q3DBars::addDataRow(QDataRow *dataRow)
{
    QDataRow *row = dataRow;
    // Check that the input data fits into sample space, and resize if it doesn't
    row->d_ptr->verifySize(d_ptr->m_sampleCount.x());
    d_ptr->m_heightNormalizer = row->d_ptr->highestValue();
    // With each new row, the previous data row must be moved back
    // ie. we need as many vectors as we have rows in the sample space
    d_ptr->m_dataSetTest->addRow(row);
    // if the added data pushed us over sample space, remove the oldest data set
    d_ptr->m_dataSetTest->d_ptr->verifySize(d_ptr->m_sampleCount.y());
}

void Q3DBars::addDataSet(const QVector< QVector<float> > &data, const QVector<QString> &labelsRow
                         , const QVector<QString> &labelsColumn)
{
    d_ptr->clearDataSet();
    // Check sizes
    if (data.at(0).size() > d_ptr->m_sampleCount.x()) {
        qCritical("Too much data per row, aborting");
        // TODO: Implement a function that goes through all rows and crops the column count
        return;
    }
    for (int i = 0; i < data.size(); i++)
        addDataRow(data.at(i));

    if (d_ptr->m_dataSet.size() > d_ptr->m_sampleCount.y()) {
        qWarning("Data set too large for sample space. Cropping it to fit.");
        d_ptr->m_dataSet.resize(d_ptr->m_sampleCount.y());
    }
}

void Q3DBars::addDataSet(const QVector< QVector<QDataItem*> > &data
                         , const QVector<QString> &labelsRow
                         , const QVector<QString> &labelsColumn)
{
    d_ptr->clearDataSet();
    // Check sizes
    if (data.at(0).size() > d_ptr->m_sampleCount.x()) {
        qCritical("Too much data per row, aborting");
        // TODO: Implement a function that goes through all rows and crops the column count
        return;
    }

    d_ptr->m_dataSet = data;

    if (d_ptr->m_dataSet.size() > d_ptr->m_sampleCount.y()) {
        qWarning("Data set too large for sample space. Cropping it to fit.");
        d_ptr->resizeDataSet();
    }

    for (int i = 0; i < d_ptr->m_dataSet.size(); i++)
        d_ptr->findHighestValue(d_ptr->m_dataSet.at(i));
}

void Q3DBars::addDataSet(QDataSet* dataSet)
{
    delete d_ptr->m_dataSetTest;
    // Check sizes
    dataSet->d_ptr->verifySize(d_ptr->m_sampleCount.y(), d_ptr->m_sampleCount.x());
    // Take ownership of given set
    d_ptr->m_dataSetTest = dataSet;
    // Find highest value
    d_ptr->m_heightNormalizer = d_ptr->m_dataSetTest->d_ptr->highestValue();
}

Q3DBarsPrivate::Q3DBarsPrivate(Q3DBars *q)
    : q_ptr(q)
    , m_paintDevice(0)
    , m_barShader(0)
    , m_selectionShader(0)
    , m_backgroundShader(0)
    , m_barObj(0)
    , m_backgroundObj(0)
    , m_sampleCount(QPoint(0, 0))
    , m_objFile(QStringLiteral(":/defaultMeshes/bar"))
    , m_mousePressed(false)
    , m_mousePos(QPoint(0, 0))
    , m_zoomLevel(100)
    , m_horizontalRotation(-45.0f)
    , m_verticalRotation(15.0f)
    , m_barThickness(QPointF(0.75f, 0.75f))
    , m_barSpacing(m_barThickness * 3.0f)
    , m_dataSet(0)
    , m_heightNormalizer(0.0f)
    , m_rowWidth(0)
    , m_columnDepth(0)
    , m_maxDimension(0)
    , m_scaleX(0)
    , m_scaleZ(0)
    , m_scaleFactorX(0)
    , m_scaleFactorZ(0)
    , m_sceneScale(0)
    , m_maxSceneSize(40.0)
    , m_baseColor(QColor(Qt::gray))
    , m_heightColor(QColor(Qt::white))
    , m_depthColor(QColor(Qt::darkGray))
    , m_backgroundColor(QColor(Qt::gray))
    , m_windowColor(QColor(Qt::gray))
    , m_textColor(QColor(Qt::white))
    , m_textBackgroundColor(QColor(Qt::black))
    , m_highlightBarColor(QColor(Qt::red))
    , m_highlightRowColor(QColor(Qt::darkRed))
    , m_highlightColumnColor(QColor(Qt::darkMagenta))
    , m_lightStrength(4.0f)
    , m_ambientStrength(0.3f)
    , m_highlightLightStrength(8.0f)
    , m_uniformColor(true)
    , m_isInitialized(false)
    , m_selectionMode(Q3DBars::Bar)
    , m_selectedBar(0)
    , m_dataSetTest(0)
{
}

Q3DBarsPrivate::~Q3DBarsPrivate()
{
    qDebug() << "Destroying Q3DBarsPrivate";
    clearDataSet();
    delete m_barShader;
    delete m_selectionShader;
    delete m_backgroundShader;
    delete m_barObj;
    delete m_backgroundObj;

#ifndef USE_HAX0R_SELECTION
    q_ptr->glDeleteFramebuffers(1, &m_framebufferSelection);
    q_ptr->glDeleteTextures(1, &m_selectionTexture);
    q_ptr->glDeleteTextures(1, &m_depthTexture);
#endif
}

void Q3DBarsPrivate::findHighestValue(const QVector<QDataItem*> &row)
{
    for (int i = 0; i < row.size(); i++) {
        QDataItem *item = row.at(i);
        float itemValue = item->d_ptr->value();
        if (m_heightNormalizer < itemValue)
            m_heightNormalizer = itemValue;
    }
}

void Q3DBarsPrivate::resizeDataSet()
{
    qDebug("resizeDataSet");
    // QVector's resize doesn't delete data contained in it
    // Delete contents of rows to be removed
    int nbrToBeRemoved = m_dataSet.size() - m_sampleCount.y();
    for (int rowCount = 0; rowCount < nbrToBeRemoved; rowCount++) {
        int rowToBeRemoved = m_dataSet.size() - rowCount - 1; // -1 to compensate index 0
        QVector<QDataItem*> row = m_dataSet.at(rowToBeRemoved);
        for (int itemCount = 0; itemCount < row.size(); itemCount++) {
            delete row.at(itemCount);
        }
        row.clear();
    }
    // Resize vector
    m_dataSet.resize(m_sampleCount.y());
}

void Q3DBarsPrivate::resizeDataRow(QVector<QDataItem *> *row)
{
    qDebug("resizeDataRow");
    // QVector's resize doesn't delete data contained in it
    // Delete contents of items to be removed
    int nbrToBeRemoved = row->size() - m_sampleCount.x();
    for (int itemCount = 0; itemCount < nbrToBeRemoved; itemCount++) {
        int itemToBeRemoved = row->size() - itemCount - 1; // -1 to compensate index 0
        delete row->at(itemToBeRemoved);
    }
    // Resize vector
    row->resize(m_sampleCount.x());
}

void Q3DBarsPrivate::clearDataSet()
{
    qDebug("clearDataSet");
    // QVector's clear doesn't delete data contained in it
    // Delete contents
    for (int rowCount = 0; rowCount < m_dataSet.size(); rowCount++) {
        QVector<QDataItem*> row = m_dataSet.at(rowCount);
        for (int itemCount = 0; itemCount < row.size(); itemCount++) {
            delete row.at(itemCount);
        }
        row.clear();
    }
    // Clear vector
    m_dataSet.clear();
}

void Q3DBarsPrivate::loadBarMesh()
{
    if (m_barObj)
        delete m_barObj;
    m_barObj = new ObjectHelper(q_ptr, m_objFile);
    m_barObj->load();
}

void Q3DBarsPrivate::loadBackgroundMesh()
{
    if (m_backgroundObj)
        delete m_backgroundObj;
    m_backgroundObj = new ObjectHelper(q_ptr, QStringLiteral(":/defaultMeshes/background"));
    m_backgroundObj->load();
}

void Q3DBarsPrivate::initShaders(const QString &vertexShader, const QString &fragmentShader)
{
    if (m_barShader)
        delete m_barShader;
    m_barShader = new ShaderHelper(q_ptr, vertexShader, fragmentShader);
    m_barShader->initialize();
}

void Q3DBarsPrivate::initSelectionShader()
{
    if (m_selectionShader)
        delete m_selectionShader;
    m_selectionShader = new ShaderHelper(q_ptr, QStringLiteral(":/shaders/vertexSelection")
                                         , QStringLiteral(":/shaders/fragmentSelection"));
    m_selectionShader->initialize();
}

void Q3DBarsPrivate::initSelectionBuffer()
{
#if 0
    // Create frame buffer object
    q_ptr->glGenFramebuffers(1, &m_framebufferSelection);
    q_ptr->glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferSelection);

    // Create texture object for the primitive information buffer
    q_ptr->glGenTextures(1, &m_selectionTexture);
    q_ptr->glBindTexture(GL_TEXTURE_2D, m_selectionTexture);
    q_ptr->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, q_ptr->width(), q_ptr->height(),
                        0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    q_ptr->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                  m_selectionTexture, 0);

    // Create texture object for the depth buffer
    q_ptr->glGenTextures(1, &m_depthTexture);
    q_ptr->glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    q_ptr->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, q_ptr->width(), q_ptr->height(),
                        0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    q_ptr->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                                  m_depthTexture, 0);

    // Verify that the frame buffer is complete
    GLenum status = q_ptr->glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        qCritical() << "Frame buffer creation failed" << status;
        return;
    }

    // Restore the default framebuffer
    q_ptr->glBindTexture(GL_TEXTURE_2D, 0);
    q_ptr->glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}

void Q3DBarsPrivate::initBackgroundShaders(const QString &vertexShader
                                           , const QString &fragmentShader)
{
    if (m_backgroundShader)
        delete m_backgroundShader;
    m_backgroundShader = new ShaderHelper(q_ptr, vertexShader, fragmentShader);
    m_backgroundShader->initialize();
}

void Q3DBarsPrivate::calculateSceneScalingFactors()
{
    // Calculate scene scaling and translation factors
    m_rowWidth = ((m_sampleCount.x() + 1) * m_barSpacing.x()) / 2.0f;
    m_columnDepth = ((m_sampleCount.y() + 1) * m_barSpacing.y()) / 2.0f;
    m_maxDimension = qMax(m_rowWidth, m_columnDepth);//(m_rowWidth > m_columnDepth) ? m_rowWidth : m_columnDepth;
    m_scaleX = m_barThickness.x() / m_sampleCount.x() * (m_maxSceneSize / m_maxDimension);
    m_scaleZ = m_barThickness.y() / m_sampleCount.x() * (m_maxSceneSize / m_maxDimension);
    m_sceneScale = qMin(m_scaleX, m_scaleZ);//(m_scaleX < m_scaleZ) ? m_scaleX : m_scaleZ;
    float minThickness = qMin(m_barThickness.x(), m_barThickness.y());//(m_barThickness.x() < m_barThickness.y()) ? m_barThickness.x() : m_barThickness.y();
    m_sceneScale = m_sceneScale / minThickness;
    m_scaleFactorX = m_sampleCount.x() * (m_maxDimension / m_maxSceneSize);
    m_scaleFactorZ = m_sampleCount.x() * (m_maxDimension / m_maxSceneSize);
    qDebug() << "m_scaleX" << m_scaleX << "m_scaleFactorX" << m_scaleFactorX;
    qDebug() << "m_scaleZ" << m_scaleZ << "m_scaleFactorZ" << m_scaleFactorZ;
    qDebug() << "m_rowWidth:" << m_rowWidth << "m_columnDepth:" << m_columnDepth << "m_maxDimension:" << m_maxDimension;
    qDebug() << m_rowWidth * m_sceneScale << m_columnDepth * m_sceneScale;
}

Q3DBarsPrivate::SelectionType Q3DBarsPrivate::isSelected(int row, int bar, const QVector3D &selection)
{
    SelectionType isSelectedType = None;
    if (selection == Utils::vectorFromColor(m_windowColor))
        return isSelectedType; // skip window
    QVector3D current = QVector3D((GLubyte)(((float)(row + 2) / (float)(m_sampleCount.y() + 2))
                                            * 255 + 0.49) // add 0.49 to fix rounding
                                  , (GLubyte)(((float)(bar + 2) / (float)(m_sampleCount.x() + 2))
                                              * 255 + 0.49) // add 0.49 to fix rounding
                                  , 0);
    //if (m_mousePressed)
    //    qDebug() << "selected:" << selection << "current:" << current;
    if (current == selection) {
        isSelectedType = Bar;
    }
    else if (current.y() == selection.y() && (m_selectionMode == Q3DBars::BarAndColumn
                                              || m_selectionMode == Q3DBars::BarRowAndColumn)) {
        isSelectedType = Column;
    }
    else if (current.x() == selection.x() && (m_selectionMode == Q3DBars::BarAndRow
                                              || m_selectionMode == Q3DBars::BarRowAndColumn)) {
        isSelectedType = Row;
    }
    return isSelectedType;
}

QTCOMMERCIALDATAVIS3D_END_NAMESPACE
