/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef TEXTUREHELPER_P_H
#define TEXTUREHELPER_P_H

#include "qdatavis3dglobal.h"
#include <QOpenGLFunctions>
#include <QRgb>

QT_DATAVIS3D_BEGIN_NAMESPACE

class TextureHelper : protected QOpenGLFunctions
{
    public:
    TextureHelper();
    ~TextureHelper();

    // Ownership of created texture is transferred to caller
    GLuint create2DTexture(const QImage &image, bool useTrilinearFiltering = false,
                           bool convert = true);
    GLuint createCubeMapTexture(const QImage &image, bool useTrilinearFiltering = false);
    // Returns selection framebuffer and inserts generated texture id to texture parameters
    GLuint createSelectionBuffer(const QSize &size, GLuint &texture, GLuint &depthTexture);
    // Returns selection texture and inserts generated framebuffers to framebuffer parameters
    GLuint createSelectionTexture(const QSize &size, GLuint &frameBuffer, GLuint &depthBuffer);
#if !defined(QT_OPENGL_ES_2)
    // Returns depth texture and inserts generated framebuffer to parameter
    GLuint createDepthTexture(const QSize &size, GLuint &frameBuffer, GLuint textureSize = 1);
#endif
    void deleteTexture(const GLuint *texture);

    private:
    QImage convertToGLFormat(const QImage &srcImage);
    void convertToGLFormatHelper(QImage &dstImage, const QImage &srcImage, GLenum texture_format);
    QRgb qt_gl_convertToGLFormatHelper(QRgb src_pixel, GLenum texture_format);

    friend class Q3DBarsPrivate;
    friend class Q3DMapsPrivate;
};

QT_DATAVIS3D_END_NAMESPACE

#endif
