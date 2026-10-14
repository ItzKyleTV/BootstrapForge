/****************************************************************************
** Meta object code from reading C++ file 'EditorManager.hpp'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../Client/include/BootstrapForge/Client/Editor/EditorManager.hpp"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'EditorManager.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN14BootstrapForge6Client6Editor13EditorManagerE_t {};
} // unnamed namespace

template <> constexpr inline auto BootstrapForge::Client::Editor::EditorManager::qt_create_metaobjectdata<qt_meta_tag_ZN14BootstrapForge6Client6Editor13EditorManagerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BootstrapForge::Client::Editor::EditorManager",
        "DocumentOpened",
        "",
        "path",
        "DocumentSaved",
        "DocumentModifiedChanged",
        "modified",
        "CurrentDocumentChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'DocumentOpened'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'DocumentSaved'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'DocumentModifiedChanged'
        QtMocHelpers::SignalData<void(const QString &, bool)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 }, { QMetaType::Bool, 6 },
        }}),
        // Signal 'CurrentDocumentChanged'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<EditorManager, qt_meta_tag_ZN14BootstrapForge6Client6Editor13EditorManagerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BootstrapForge::Client::Editor::EditorManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BootstrapForge6Client6Editor13EditorManagerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BootstrapForge6Client6Editor13EditorManagerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14BootstrapForge6Client6Editor13EditorManagerE_t>.metaTypes,
    nullptr
} };

void BootstrapForge::Client::Editor::EditorManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<EditorManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->DocumentOpened((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->DocumentSaved((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->DocumentModifiedChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        case 3: _t->CurrentDocumentChanged((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (EditorManager::*)(const QString & )>(_a, &EditorManager::DocumentOpened, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (EditorManager::*)(const QString & )>(_a, &EditorManager::DocumentSaved, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (EditorManager::*)(const QString & , bool )>(_a, &EditorManager::DocumentModifiedChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (EditorManager::*)(const QString & )>(_a, &EditorManager::CurrentDocumentChanged, 3))
            return;
    }
}

const QMetaObject *BootstrapForge::Client::Editor::EditorManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BootstrapForge::Client::Editor::EditorManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BootstrapForge6Client6Editor13EditorManagerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BootstrapForge::Client::Editor::EditorManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void BootstrapForge::Client::Editor::EditorManager::DocumentOpened(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void BootstrapForge::Client::Editor::EditorManager::DocumentSaved(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void BootstrapForge::Client::Editor::EditorManager::DocumentModifiedChanged(const QString & _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2);
}

// SIGNAL 3
void BootstrapForge::Client::Editor::EditorManager::CurrentDocumentChanged(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
