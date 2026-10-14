/****************************************************************************
** Meta object code from reading C++ file 'CMakeProjectController.hpp'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../Client/include/BootstrapForge/Client/Build/CMakeProjectController.hpp"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CMakeProjectController.hpp' doesn't include <QObject>."
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
struct qt_meta_tag_ZN14BootstrapForge6Client5Build22CMakeProjectControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto BootstrapForge::Client::Build::CMakeProjectController::qt_create_metaobjectdata<qt_meta_tag_ZN14BootstrapForge6Client5Build22CMakeProjectControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BootstrapForge::Client::Build::CMakeProjectController",
        "OutputReceived",
        "",
        "text",
        "TestsDiscovered",
        "testNames",
        "BusyChanged",
        "busy",
        "OperationFinished",
        "operation",
        "succeeded"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'OutputReceived'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'TestsDiscovered'
        QtMocHelpers::SignalData<void(const QStringList &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QStringList, 5 },
        }}),
        // Signal 'BusyChanged'
        QtMocHelpers::SignalData<void(bool)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 7 },
        }}),
        // Signal 'OperationFinished'
        QtMocHelpers::SignalData<void(const QString &, bool)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 }, { QMetaType::Bool, 10 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<CMakeProjectController, qt_meta_tag_ZN14BootstrapForge6Client5Build22CMakeProjectControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BootstrapForge::Client::Build::CMakeProjectController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BootstrapForge6Client5Build22CMakeProjectControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BootstrapForge6Client5Build22CMakeProjectControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14BootstrapForge6Client5Build22CMakeProjectControllerE_t>.metaTypes,
    nullptr
} };

void BootstrapForge::Client::Build::CMakeProjectController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<CMakeProjectController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->OutputReceived((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->TestsDiscovered((*reinterpret_cast<std::add_pointer_t<QStringList>>(_a[1]))); break;
        case 2: _t->BusyChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->OperationFinished((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (CMakeProjectController::*)(const QString & )>(_a, &CMakeProjectController::OutputReceived, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (CMakeProjectController::*)(const QStringList & )>(_a, &CMakeProjectController::TestsDiscovered, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (CMakeProjectController::*)(bool )>(_a, &CMakeProjectController::BusyChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (CMakeProjectController::*)(const QString & , bool )>(_a, &CMakeProjectController::OperationFinished, 3))
            return;
    }
}

const QMetaObject *BootstrapForge::Client::Build::CMakeProjectController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BootstrapForge::Client::Build::CMakeProjectController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BootstrapForge6Client5Build22CMakeProjectControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BootstrapForge::Client::Build::CMakeProjectController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void BootstrapForge::Client::Build::CMakeProjectController::OutputReceived(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void BootstrapForge::Client::Build::CMakeProjectController::TestsDiscovered(const QStringList & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void BootstrapForge::Client::Build::CMakeProjectController::BusyChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void BootstrapForge::Client::Build::CMakeProjectController::OperationFinished(const QString & _t1, bool _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2);
}
QT_WARNING_POP
