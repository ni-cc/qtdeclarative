/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the demonstration applications of the Qt Toolkit.
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

#ifndef CUBEVIEW_H
#define CUBEVIEW_H

#include <QtQuick/QQuickItem>
#include <QtGui/QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

//! [1]
class CubeView : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(qreal t READ t WRITE setT NOTIFY tChanged)
    Q_PROPERTY(float ScrollerWidth READ ScrollerWidth)
    Q_PROPERTY(float ScrollerHeight READ ScrollerHeight)
    Q_PROPERTY(qreal scrollOffsetX READ scrollOffsetX WRITE setScrollOffsetX NOTIFY scrollOffsetXChanged)

public:
    CubeView();

    qreal t() const { return m_t; }
    void setT(qreal t);

    qreal scrollOffsetX() const { return m_scrollOffsetX; }
    void setScrollOffsetX(qreal scrollOffsetX);

    float ScrollerWidth() const { return LittleCubeWidth*NumLittleCubes; }
    float ScrollerHeight() const { return m_ScrollerHeight; }

signals:
    void tChanged();
    void scrollOffsetXChanged();

public slots:
    void paint();
    void cleanup();
    void sync();
    void randomizeBigCube();
    void handleLittleCubeTap(int x);

private slots:
    void handleWindowChanged(QQuickWindow *win);

private:

    void UpdateCubes ();
    float UpdatedRotationRadians (float radians, float speed, double elapsedTime);

    const int NumLittleCubes = 20;
    const float LittleCubeWidth = (320 / 3);
    const float m_ScrollerHeight = LittleCubeWidth;
    const float UnitLittleCubeWidth = 2;

    QOpenGLVertexArrayObject *vertexArray;
    QOpenGLBuffer *vertexBuffer;

    struct CubeInfo
    {
        float Red;
        float Green;
        float Blue;
        float XAxis;
        float YAxis;
        float ZAxis;
        float Speed;
        float RotationRadians;

        CubeInfo (float r = 0.0f, float g = 0.0f, float b = 0.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f, float s = 0.0f, float rr = 0.0f)
        {
            Red = r;
            Green = g;
            Blue = b;
            XAxis = x;
            YAxis = y;
            ZAxis = z;
            Speed = s;
            RotationRadians = rr;
        }
    };

    CubeInfo littleCube[20];
    CubeInfo bigCube;
    CubeInfo bigCubeDirections;

    CubeInfo minimums;
    CubeInfo maximums;
    CubeInfo deltas;

    double timeOfLastRenderedFrame;
    qreal m_scrollOffsetX;

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    QOpenGLShaderProgram *m_program;

    qreal m_t;
    qreal m_thread_t;
};
//! [1]

#endif // CUBEVIEW_H
