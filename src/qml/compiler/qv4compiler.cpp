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

#include <qv4compiler_p.h>
#include <qv4compileddata_p.h>
#include <qv4isel_p.h>
#include <qv4engine_p.h>
#include <private/qqmlpropertycache_p.h>

QV4::Compiler::StringTableGenerator::StringTableGenerator()
{
    clear();
}

int QV4::Compiler::StringTableGenerator::registerString(const QString &str)
{
    QHash<QString, int>::ConstIterator it = stringToId.find(str);
    if (it != stringToId.end())
        return *it;
    stringToId.insert(str, strings.size());
    strings.append(str);
    stringDataSize += QV4::CompiledData::String::calculateSize(str);
    return strings.size() - 1;
}

int QV4::Compiler::StringTableGenerator::getStringId(const QString &string) const
{
    Q_ASSERT(stringToId.contains(string));
    return stringToId.value(string);
}

void QV4::Compiler::StringTableGenerator::clear()
{
    strings.clear();
    stringToId.clear();
    stringDataSize = 0;
}

void QV4::Compiler::StringTableGenerator::serialize(uint *stringTable, char *dataStart, char *stringData)
{
    for (int i = 0; i < strings.size(); ++i) {
        stringTable[i] = stringData - dataStart;
        const QString &qstr = strings.at(i);

        QV4::CompiledData::String *s = (QV4::CompiledData::String*)(stringData);
        s->hash = QV4::String::createHashValue(qstr.constData(), qstr.length());
        s->flags = 0; // ###
        s->str.ref.atomic.store(-1);
        s->str.size = qstr.length();
        s->str.alloc = 0;
        s->str.capacityReserved = false;
        s->str.offset = sizeof(QArrayData);
        memcpy(s + 1, qstr.constData(), (qstr.length() + 1)*sizeof(ushort));

        stringData += QV4::CompiledData::String::calculateSize(qstr);
    }
}

QV4::Compiler::JSUnitGenerator::JSUnitGenerator(QV4::IR::Module *module, int headerSize)
    : irModule(module)
    , jsClassDataSize(0)
{
    if (headerSize == -1)
        headerSize = sizeof(QV4::CompiledData::Unit);
    this->headerSize = headerSize;
    // Make sure the empty string always gets index 0
    registerString(QString());
}

uint QV4::Compiler::JSUnitGenerator::registerIndexedGetterLookup()
{
    CompiledData::Lookup l;
    l.type_and_flags = CompiledData::Lookup::Type_IndexedGetter;
    l.nameIndex = 0;
    lookups << l;
    return lookups.size() - 1;
}

uint QV4::Compiler::JSUnitGenerator::registerIndexedSetterLookup()
{
    CompiledData::Lookup l;
    l.type_and_flags = CompiledData::Lookup::Type_IndexedSetter;
    l.nameIndex = 0;
    lookups << l;
    return lookups.size() - 1;
}

uint QV4::Compiler::JSUnitGenerator::registerGetterLookup(const QString &name)
{
    CompiledData::Lookup l;
    l.type_and_flags = CompiledData::Lookup::Type_Getter;
    l.nameIndex = registerString(name);
    lookups << l;
    return lookups.size() - 1;
}


uint QV4::Compiler::JSUnitGenerator::registerSetterLookup(const QString &name)
{
    CompiledData::Lookup l;
    l.type_and_flags = CompiledData::Lookup::Type_Setter;
    l.nameIndex = registerString(name);
    lookups << l;
    return lookups.size() - 1;
}

uint QV4::Compiler::JSUnitGenerator::registerGlobalGetterLookup(const QString &name)
{
    CompiledData::Lookup l;
    l.type_and_flags = CompiledData::Lookup::Type_GlobalGetter;
    l.nameIndex = registerString(name);
    lookups << l;
    return lookups.size() - 1;
}

int QV4::Compiler::JSUnitGenerator::registerRegExp(QV4::IR::RegExp *regexp)
{
    CompiledData::RegExp re;
    re.stringIndex = registerString(*regexp->value);

    re.flags = 0;
    if (regexp->flags & QV4::IR::RegExp::RegExp_Global)
        re.flags |= CompiledData::RegExp::RegExp_Global;
    if (regexp->flags & QV4::IR::RegExp::RegExp_IgnoreCase)
        re.flags |= CompiledData::RegExp::RegExp_IgnoreCase;
    if (regexp->flags & QV4::IR::RegExp::RegExp_Multiline)
        re.flags |= CompiledData::RegExp::RegExp_Multiline;

    regexps.append(re);
    return regexps.size() - 1;
}

int QV4::Compiler::JSUnitGenerator::registerConstant(QV4::ReturnedValue v)
{
    int idx = constants.indexOf(v);
    if (idx >= 0)
        return idx;
    constants.append(v);
    return constants.size() - 1;
}

int QV4::Compiler::JSUnitGenerator::registerJSClass(int count, IR::ExprList *args)
{
    // ### re-use existing class definitions.

    QList<CompiledData::JSClassMember> members;

    IR::ExprList *it = args;
    for (int i = 0; i < count; ++i, it = it->next) {
        CompiledData::JSClassMember member;

        QV4::IR::Name *name = it->expr->asName();
        it = it->next;

        const bool isData = it->expr->asConst()->value;
        it = it->next;

        member.nameOffset = registerString(*name->id);
        member.isAccessor = !isData;
        members << member;

        if (!isData)
            it = it->next;
    }

    jsClasses << members;
    jsClassDataSize += CompiledData::JSClass::calculateSize(members.count());
    return jsClasses.size() - 1;
}

QV4::CompiledData::Unit *QV4::Compiler::JSUnitGenerator::generateUnit()
{
    registerString(irModule->fileName);
    foreach (QV4::IR::Function *f, irModule->functions) {
        registerString(*f->name);
        for (int i = 0; i < f->formals.size(); ++i)
            registerString(*f->formals.at(i));
        for (int i = 0; i < f->locals.size(); ++i)
            registerString(*f->locals.at(i));
    }

    int unitSize = QV4::CompiledData::Unit::calculateSize(headerSize, irModule->functions.size(), regexps.size(),
                                                          constants.size(), lookups.size(), jsClasses.count());

    uint functionDataSize = 0;
    for (int i = 0; i < irModule->functions.size(); ++i) {
        QV4::IR::Function *f = irModule->functions.at(i);
        functionOffsets.insert(f, functionDataSize + unitSize);

        const int qmlIdDepsCount = f->idObjectDependencies.count();
        const int qmlPropertyDepsCount = f->scopeObjectPropertyDependencies.count() + f->contextObjectPropertyDependencies.count();
        functionDataSize += QV4::CompiledData::Function::calculateSize(f->formals.size(), f->locals.size(), f->nestedFunctions.size(), qmlIdDepsCount, qmlPropertyDepsCount);
    }

    const int totalSize = unitSize + functionDataSize + jsClassDataSize + stringTable.sizeOfTableAndData();
    char *data = (char *)malloc(totalSize);
    memset(data, 0, totalSize);
    QV4::CompiledData::Unit *unit = (QV4::CompiledData::Unit*)data;

    memcpy(unit->magic, QV4::CompiledData::magic_str, sizeof(unit->magic));
    unit->architecture = 0; // ###
    unit->flags = QV4::CompiledData::Unit::IsJavascript;
    unit->version = 1;
    unit->unitSize = totalSize;
    unit->functionTableSize = irModule->functions.size();
    unit->offsetToFunctionTable = headerSize;
    unit->lookupTableSize = lookups.count();
    unit->offsetToLookupTable = unit->offsetToFunctionTable + unit->functionTableSize * sizeof(uint);
    unit->regexpTableSize = regexps.size();
    unit->offsetToRegexpTable = unit->offsetToLookupTable + unit->lookupTableSize * CompiledData::Lookup::calculateSize();
    unit->constantTableSize = constants.size();
    unit->offsetToConstantTable = unit->offsetToRegexpTable + unit->regexpTableSize * CompiledData::RegExp::calculateSize();
    unit->jsClassTableSize = jsClasses.count();
    unit->offsetToJSClassTable = unit->offsetToConstantTable + unit->constantTableSize * sizeof(ReturnedValue);
    unit->stringTableSize = stringTable.stringCount();
    unit->offsetToStringTable = unitSize + functionDataSize + jsClassDataSize;
    unit->indexOfRootFunction = -1;
    unit->sourceFileIndex = getStringId(irModule->fileName);

    uint *functionTable = (uint *)(data + unit->offsetToFunctionTable);
    for (int i = 0; i < irModule->functions.size(); ++i)
        functionTable[i] = functionOffsets.value(irModule->functions.at(i));

    char *f = data + unitSize;
    for (int i = 0; i < irModule->functions.size(); ++i) {
        QV4::IR::Function *function = irModule->functions.at(i);
        if (function == irModule->rootFunction)
            unit->indexOfRootFunction = i;

        const int bytes = writeFunction(f, i, function);
        f += bytes;
    }

    CompiledData::Lookup *lookupsToWrite = (CompiledData::Lookup*)(data + unit->offsetToLookupTable);
    foreach (const CompiledData::Lookup &l, lookups)
        *lookupsToWrite++ = l;

    CompiledData::RegExp *regexpTable = (CompiledData::RegExp *)(data + unit->offsetToRegexpTable);
    memcpy(regexpTable, regexps.constData(), regexps.size() * sizeof(*regexpTable));

    ReturnedValue *constantTable = (ReturnedValue *)(data + unit->offsetToConstantTable);
    memcpy(constantTable, constants.constData(), constants.size() * sizeof(ReturnedValue));

    // write js classes and js class lookup table
    uint *jsClassTable = (uint*)(data + unit->offsetToJSClassTable);
    char *jsClass = data + unitSize + functionDataSize;
    for (int i = 0; i < jsClasses.count(); ++i) {
        jsClassTable[i] = jsClass - data;

        const QList<CompiledData::JSClassMember> members = jsClasses.at(i);

        CompiledData::JSClass *c = reinterpret_cast<CompiledData::JSClass*>(jsClass);
        c->nMembers = members.count();

        CompiledData::JSClassMember *memberToWrite = reinterpret_cast<CompiledData::JSClassMember*>(jsClass + sizeof(CompiledData::JSClass));
        foreach (const CompiledData::JSClassMember &member, members)
            *memberToWrite++ = member;

        jsClass += CompiledData::JSClass::calculateSize(members.count());
    }

    // write strings and string table
    {
        uint *stringTablePtr = (uint *)(data + unit->offsetToStringTable);
        char *string = data + unit->offsetToStringTable + unit->stringTableSize * sizeof(uint);
        stringTable.serialize(stringTablePtr, data, string);
    }

    return unit;
}

int QV4::Compiler::JSUnitGenerator::writeFunction(char *f, int index, QV4::IR::Function *irFunction)
{
    QV4::CompiledData::Function *function = (QV4::CompiledData::Function *)f;

    quint32 currentOffset = sizeof(QV4::CompiledData::Function);

    function->index = index;
    function->nameIndex = getStringId(*irFunction->name);
    function->flags = 0;
    if (irFunction->hasDirectEval)
        function->flags |= CompiledData::Function::HasDirectEval;
    if (irFunction->usesArgumentsObject)
        function->flags |= CompiledData::Function::UsesArgumentsObject;
    if (irFunction->isStrict)
        function->flags |= CompiledData::Function::IsStrict;
    if (irFunction->isNamedExpression)
        function->flags |= CompiledData::Function::IsNamedExpression;
    if (irFunction->hasTry || irFunction->hasWith)
        function->flags |= CompiledData::Function::HasCatchOrWith;
    function->nFormals = irFunction->formals.size();
    function->formalsOffset = currentOffset;
    currentOffset += function->nFormals * sizeof(quint32);

    function->nLocals = irFunction->locals.size();
    function->localsOffset = currentOffset;
    currentOffset += function->nLocals * sizeof(quint32);

    function->nInnerFunctions = irFunction->nestedFunctions.size();
    function->innerFunctionsOffset = currentOffset;
    currentOffset += function->nInnerFunctions * sizeof(quint32);

    function->nDependingIdObjects = 0;
    function->nDependingContextProperties = 0;
    function->nDependingScopeProperties = 0;

    if (!irFunction->idObjectDependencies.isEmpty()) {
        function->nDependingIdObjects = irFunction->idObjectDependencies.count();
        function->dependingIdObjectsOffset = currentOffset;
        currentOffset += function->nDependingIdObjects * sizeof(quint32);
    }

    if (!irFunction->contextObjectPropertyDependencies.isEmpty()) {
        function->nDependingContextProperties = irFunction->contextObjectPropertyDependencies.count();
        function->dependingContextPropertiesOffset = currentOffset;
        currentOffset += function->nDependingContextProperties * sizeof(quint32) * 2;
    }

    if (!irFunction->scopeObjectPropertyDependencies.isEmpty()) {
        function->nDependingScopeProperties = irFunction->scopeObjectPropertyDependencies.count();
        function->dependingScopePropertiesOffset = currentOffset;
        currentOffset += function->nDependingScopeProperties * sizeof(quint32) * 2;
    }

    function->location.line = irFunction->line;
    function->location.column = irFunction->column;

    // write formals
    quint32 *formals = (quint32 *)(f + function->formalsOffset);
    for (int i = 0; i < irFunction->formals.size(); ++i)
        formals[i] = getStringId(*irFunction->formals.at(i));

    // write locals
    quint32 *locals = (quint32 *)(f + function->localsOffset);
    for (int i = 0; i < irFunction->locals.size(); ++i)
        locals[i] = getStringId(*irFunction->locals.at(i));

    // write inner functions
    quint32 *innerFunctions = (quint32 *)(f + function->innerFunctionsOffset);
    for (int i = 0; i < irFunction->nestedFunctions.size(); ++i)
        innerFunctions[i] = functionOffsets.value(irFunction->nestedFunctions.at(i));

    // write QML dependencies
    quint32 *writtenDeps = (quint32 *)(f + function->dependingIdObjectsOffset);
    foreach (int id, irFunction->idObjectDependencies)
        *writtenDeps++ = id;

    writtenDeps = (quint32 *)(f + function->dependingContextPropertiesOffset);
    for (QV4::IR::PropertyDependencyMap::ConstIterator property = irFunction->contextObjectPropertyDependencies.constBegin(), end = irFunction->contextObjectPropertyDependencies.constEnd();
         property != end; ++property) {
        *writtenDeps++ = property.key(); // property index
        *writtenDeps++ = property.value(); // notify index
    }

    writtenDeps = (quint32 *)(f + function->dependingScopePropertiesOffset);
    for (QV4::IR::PropertyDependencyMap::ConstIterator property = irFunction->scopeObjectPropertyDependencies.constBegin(), end = irFunction->scopeObjectPropertyDependencies.constEnd();
         property != end; ++property) {
        *writtenDeps++ = property.key(); // property index
        *writtenDeps++ = property.value(); // notify index
    }

    return CompiledData::Function::calculateSize(function->nFormals, function->nLocals, function->nInnerFunctions,
                                                 function->nDependingIdObjects, function->nDependingContextProperties + function->nDependingScopeProperties);
}
