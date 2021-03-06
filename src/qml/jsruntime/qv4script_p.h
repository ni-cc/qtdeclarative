/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtQml module of the Qt Toolkit.
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
#ifndef QV4SCRIPT_H
#define QV4SCRIPT_H

#include "qv4global_p.h"
#include "qv4engine_p.h"
#include "qv4functionobject_p.h"

#include <QQmlError>

QT_BEGIN_NAMESPACE

namespace QV4 {

struct ExecutionContext;

struct QmlBindingWrapper : FunctionObject {
    V4_OBJECT

    QmlBindingWrapper(ExecutionContext *scope, Function *f, ObjectRef qml);
    // Constructor for QML functions and signal handlers, resulting binding wrapper is not callable!
    QmlBindingWrapper(ExecutionContext *scope, ObjectRef qml);

    static ReturnedValue call(Managed *that, CallData *);
    static void markObjects(Managed *m, ExecutionEngine *e);

    CallContext *context() const { return qmlContext; }

private:
    Object *qml;
    CallContext *qmlContext;
};

struct Q_QML_EXPORT Script {
    Script(ExecutionContext *scope, const QString &sourceCode, const QString &source = QString(), int line = 1, int column = 0)
        : sourceFile(source), line(line), column(column), sourceCode(sourceCode)
        , scope(scope), strictMode(false), inheritContext(false), parsed(false)
        , vmFunction(0), parseAsBinding(false) {}
    Script(ExecutionEngine *engine, ObjectRef qml, const QString &sourceCode, const QString &source = QString(), int line = 1, int column = 0)
        : sourceFile(source), line(line), column(column), sourceCode(sourceCode)
        , scope(engine->rootContext), strictMode(false), inheritContext(true), parsed(false)
        , qml(qml.asReturnedValue()), vmFunction(0), parseAsBinding(true) {}
    Script(ExecutionEngine *engine, ObjectRef qml, CompiledData::CompilationUnit *compilationUnit);
    ~Script();
    QString sourceFile;
    int line;
    int column;
    QString sourceCode;
    ExecutionContext *scope;
    bool strictMode;
    bool inheritContext;
    bool parsed;
    QV4::PersistentValue qml;
    QV4::PersistentValue compilationUnitHolder;
    Function *vmFunction;
    bool parseAsBinding;

    void parse();
    ReturnedValue run();
    ReturnedValue qmlBinding();

    Function *function();

    static QV4::CompiledData::CompilationUnit *precompile(IR::Module *module, Compiler::JSUnitGenerator *unitGenerator, ExecutionEngine *engine, const QUrl &url, const QString &source, QList<QQmlError> *reportedErrors = 0);

    static ReturnedValue evaluate(ExecutionEngine *engine, const QString &script, ObjectRef scopeObject);
};

}

QT_END_NAMESPACE

#endif
