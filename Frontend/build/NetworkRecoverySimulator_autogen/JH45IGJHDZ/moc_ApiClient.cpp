/****************************************************************************
** Meta object code from reading C++ file 'ApiClient.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../api/ApiClient.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ApiClient.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN9ApiClientE_t {};
} // unnamed namespace

template <> constexpr inline auto ApiClient::qt_create_metaobjectdata<qt_meta_tag_ZN9ApiClientE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ApiClient",
        "stateUpdated",
        "",
        "routerCount",
        "QList<Connection>",
        "connections",
        "routeReceived",
        "src",
        "dest",
        "reachable",
        "cost",
        "QList<int>",
        "route",
        "linkFailedResult",
        "u",
        "v",
        "success",
        "message",
        "linkRecoveredResult",
        "undoResult",
        "routerAddedResult",
        "routerId",
        "count",
        "edgeSavedResult",
        "weight",
        "edgeDeletedResult",
        "backendStatusChanged",
        "connected",
        "requestStatus",
        "active",
        "description"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'stateUpdated'
        QtMocHelpers::SignalData<void(int, const QList<Connection> &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 3 }, { 0x80000000 | 4, 5 },
        }}),
        // Signal 'routeReceived'
        QtMocHelpers::SignalData<void(int, int, bool, int, const QList<int> &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 }, { QMetaType::Int, 8 }, { QMetaType::Bool, 9 }, { QMetaType::Int, 10 },
            { 0x80000000 | 11, 12 },
        }}),
        // Signal 'linkFailedResult'
        QtMocHelpers::SignalData<void(int, int, bool, const QString &)>(13, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 }, { QMetaType::Int, 15 }, { QMetaType::Bool, 16 }, { QMetaType::QString, 17 },
        }}),
        // Signal 'linkRecoveredResult'
        QtMocHelpers::SignalData<void(int, int, bool, const QString &)>(18, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 14 }, { QMetaType::Int, 15 }, { QMetaType::Bool, 16 }, { QMetaType::QString, 17 },
        }}),
        // Signal 'undoResult'
        QtMocHelpers::SignalData<void(bool, const QString &)>(19, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 16 }, { QMetaType::QString, 17 },
        }}),
        // Signal 'routerAddedResult'
        QtMocHelpers::SignalData<void(bool, int, int, const QString &)>(20, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 16 }, { QMetaType::Int, 21 }, { QMetaType::Int, 22 }, { QMetaType::QString, 17 },
        }}),
        // Signal 'edgeSavedResult'
        QtMocHelpers::SignalData<void(bool, int, int, int, const QString &)>(23, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 16 }, { QMetaType::Int, 14 }, { QMetaType::Int, 15 }, { QMetaType::Int, 24 },
            { QMetaType::QString, 17 },
        }}),
        // Signal 'edgeDeletedResult'
        QtMocHelpers::SignalData<void(bool, int, int, const QString &)>(25, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 16 }, { QMetaType::Int, 14 }, { QMetaType::Int, 15 }, { QMetaType::QString, 17 },
        }}),
        // Signal 'backendStatusChanged'
        QtMocHelpers::SignalData<void(bool, const QString &)>(26, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 27 }, { QMetaType::QString, 17 },
        }}),
        // Signal 'requestStatus'
        QtMocHelpers::SignalData<void(bool, const QString &)>(28, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 29 }, { QMetaType::QString, 30 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ApiClient, qt_meta_tag_ZN9ApiClientE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ApiClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9ApiClientE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9ApiClientE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9ApiClientE_t>.metaTypes,
    nullptr
} };

void ApiClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ApiClient *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->stateUpdated((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QList<Connection>>>(_a[2]))); break;
        case 1: _t->routeReceived((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<QList<int>>>(_a[5]))); break;
        case 2: _t->linkFailedResult((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4]))); break;
        case 3: _t->linkRecoveredResult((*reinterpret_cast<std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4]))); break;
        case 4: _t->undoResult((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->routerAddedResult((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4]))); break;
        case 6: _t->edgeSavedResult((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[4])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[5]))); break;
        case 7: _t->edgeDeletedResult((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast<std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[4]))); break;
        case 8: _t->backendStatusChanged((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 9: _t->requestStatus((*reinterpret_cast<std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 4:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ApiClient::*)(int , const QList<Connection> & )>(_a, &ApiClient::stateUpdated, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApiClient::*)(int , int , bool , int , const QList<int> & )>(_a, &ApiClient::routeReceived, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApiClient::*)(int , int , bool , const QString & )>(_a, &ApiClient::linkFailedResult, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApiClient::*)(int , int , bool , const QString & )>(_a, &ApiClient::linkRecoveredResult, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApiClient::*)(bool , const QString & )>(_a, &ApiClient::undoResult, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApiClient::*)(bool , int , int , const QString & )>(_a, &ApiClient::routerAddedResult, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApiClient::*)(bool , int , int , int , const QString & )>(_a, &ApiClient::edgeSavedResult, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApiClient::*)(bool , int , int , const QString & )>(_a, &ApiClient::edgeDeletedResult, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApiClient::*)(bool , const QString & )>(_a, &ApiClient::backendStatusChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (ApiClient::*)(bool , const QString & )>(_a, &ApiClient::requestStatus, 9))
            return;
    }
}

const QMetaObject *ApiClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ApiClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9ApiClientE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ApiClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void ApiClient::stateUpdated(int _t1, const QList<Connection> & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void ApiClient::routeReceived(int _t1, int _t2, bool _t3, int _t4, const QList<int> & _t5)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1, _t2, _t3, _t4, _t5);
}

// SIGNAL 2
void ApiClient::linkFailedResult(int _t1, int _t2, bool _t3, const QString & _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 3
void ApiClient::linkRecoveredResult(int _t1, int _t2, bool _t3, const QString & _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 4
void ApiClient::undoResult(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1, _t2);
}

// SIGNAL 5
void ApiClient::routerAddedResult(bool _t1, int _t2, int _t3, const QString & _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 6
void ApiClient::edgeSavedResult(bool _t1, int _t2, int _t3, int _t4, const QString & _t5)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 6, nullptr, _t1, _t2, _t3, _t4, _t5);
}

// SIGNAL 7
void ApiClient::edgeDeletedResult(bool _t1, int _t2, int _t3, const QString & _t4)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1, _t2, _t3, _t4);
}

// SIGNAL 8
void ApiClient::backendStatusChanged(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 8, nullptr, _t1, _t2);
}

// SIGNAL 9
void ApiClient::requestStatus(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 9, nullptr, _t1, _t2);
}
QT_WARNING_POP
