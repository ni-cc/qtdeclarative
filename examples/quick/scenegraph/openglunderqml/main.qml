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

//! [1]
import QtQuick 2.0
import OpenGLScroller 1.0

Item {

    width: 320
    height: 480

//http://blog.qt.digia.com/blog/2012/08/01/scene-graph-adaptation-layer
    CubeView {
        id: cube
        SequentialAnimation on t {
            NumberAnimation { from: 0; to: 1; duration: 2500; easing.type: Easing.Linear }
   //         NumberAnimation { to: 0; duration: 2500; easing.type: Easing.OutQuad }
            loops: Animation.Infinite
            running: true
        }
        scrollOffsetX: scrollableFrame.contentX
    }
//! [1] //! [2]
    Flickable {
        id: scrollableFrame
        height: cube.ScrollerHeight+30
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.top: parent.top
        contentWidth: cube.ScrollerWidth
        contentHeight: cube.ScrollerHeight+30

        MouseArea {
            //id:
            anchors.fill: parent
            onClicked: cube.handleLittleCubeTap(mouseX)
        }
    }

    MouseArea {
        id: bigCubeFrame
        anchors.top: scrollableFrame.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        onClicked: cube.randomizeBigCube()
    }
}
//! [2]
