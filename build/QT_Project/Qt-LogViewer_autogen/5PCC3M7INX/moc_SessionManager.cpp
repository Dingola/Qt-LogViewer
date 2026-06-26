/****************************************************************************
** Meta object code from reading C++ file 'SessionManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Services/SessionManager.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SessionManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_SessionManager_t {
    uint offsetsAndSizes[14];
    char stringdata0[15];
    char stringdata1[25];
    char stringdata2[1];
    char stringdata3[27];
    char stringdata4[6];
    char stringdata5[24];
    char stringdata6[27];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_SessionManager_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_SessionManager_t qt_meta_stringdata_SessionManager = {
    {
        QT_MOC_LITERAL(0, 14),  // "SessionManager"
        QT_MOC_LITERAL(15, 24),  // "recent_log_files_changed"
        QT_MOC_LITERAL(40, 0),  // ""
        QT_MOC_LITERAL(41, 26),  // "QList<RecentLogFileRecord>"
        QT_MOC_LITERAL(68, 5),  // "items"
        QT_MOC_LITERAL(74, 23),  // "recent_sessions_changed"
        QT_MOC_LITERAL(98, 26)   // "QList<RecentSessionRecord>"
    },
    "SessionManager",
    "recent_log_files_changed",
    "",
    "QList<RecentLogFileRecord>",
    "items",
    "recent_sessions_changed",
    "QList<RecentSessionRecord>"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_SessionManager[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   26,    2, 0x06,    1 /* Public */,
       5,    1,   29,    2, 0x06,    3 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 6,    4,

       0        // eod
};

Q_CONSTINIT const QMetaObject SessionManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_SessionManager.offsetsAndSizes,
    qt_meta_data_SessionManager,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_SessionManager_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SessionManager, std::true_type>,
        // method 'recent_log_files_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<RecentLogFileRecord> &, std::false_type>,
        // method 'recent_sessions_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<RecentSessionRecord> &, std::false_type>
    >,
    nullptr
} };

void SessionManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SessionManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->recent_log_files_changed((*reinterpret_cast< std::add_pointer_t<QList<RecentLogFileRecord>>>(_a[1]))); break;
        case 1: _t->recent_sessions_changed((*reinterpret_cast< std::add_pointer_t<QList<RecentSessionRecord>>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SessionManager::*)(const QVector<RecentLogFileRecord> & );
            if (_t _q_method = &SessionManager::recent_log_files_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SessionManager::*)(const QVector<RecentSessionRecord> & );
            if (_t _q_method = &SessionManager::recent_sessions_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject *SessionManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SessionManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SessionManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SessionManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void SessionManager::recent_log_files_changed(const QVector<RecentLogFileRecord> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SessionManager::recent_sessions_changed(const QVector<RecentSessionRecord> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
