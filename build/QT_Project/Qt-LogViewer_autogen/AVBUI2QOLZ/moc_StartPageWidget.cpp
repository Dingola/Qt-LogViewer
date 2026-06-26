/****************************************************************************
** Meta object code from reading C++ file 'StartPageWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Views/App/StartPageWidget.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'StartPageWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_StartPageWidget_t {
    uint offsetsAndSizes[22];
    char stringdata0[16];
    char stringdata1[24];
    char stringdata2[1];
    char stringdata3[27];
    char stringdata4[10];
    char stringdata5[29];
    char stringdata6[23];
    char stringdata7[11];
    char stringdata8[30];
    char stringdata9[30];
    char stringdata10[25];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_StartPageWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_StartPageWidget_t qt_meta_stringdata_StartPageWidget = {
    {
        QT_MOC_LITERAL(0, 15),  // "StartPageWidget"
        QT_MOC_LITERAL(16, 23),  // "open_log_file_requested"
        QT_MOC_LITERAL(40, 0),  // ""
        QT_MOC_LITERAL(41, 26),  // "open_recent_file_requested"
        QT_MOC_LITERAL(68, 9),  // "file_path"
        QT_MOC_LITERAL(78, 28),  // "clear_recent_files_requested"
        QT_MOC_LITERAL(107, 22),  // "open_session_requested"
        QT_MOC_LITERAL(130, 10),  // "session_id"
        QT_MOC_LITERAL(141, 29),  // "open_recent_session_requested"
        QT_MOC_LITERAL(171, 29),  // "reopen_last_session_requested"
        QT_MOC_LITERAL(201, 24)   // "delete_session_requested"
    },
    "StartPageWidget",
    "open_log_file_requested",
    "",
    "open_recent_file_requested",
    "file_path",
    "clear_recent_files_requested",
    "open_session_requested",
    "session_id",
    "open_recent_session_requested",
    "reopen_last_session_requested",
    "delete_session_requested"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_StartPageWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   56,    2, 0x06,    1 /* Public */,
       3,    1,   57,    2, 0x06,    2 /* Public */,
       5,    0,   60,    2, 0x06,    4 /* Public */,
       6,    1,   61,    2, 0x06,    5 /* Public */,
       8,    1,   64,    2, 0x06,    7 /* Public */,
       9,    0,   67,    2, 0x06,    9 /* Public */,
      10,    1,   68,    2, 0x06,   10 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,

       0        // eod
};

Q_CONSTINIT const QMetaObject StartPageWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_StartPageWidget.offsetsAndSizes,
    qt_meta_data_StartPageWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_StartPageWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<StartPageWidget, std::true_type>,
        // method 'open_log_file_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'open_recent_file_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'clear_recent_files_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'open_session_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'open_recent_session_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'reopen_last_session_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'delete_session_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void StartPageWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<StartPageWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->open_log_file_requested(); break;
        case 1: _t->open_recent_file_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->clear_recent_files_requested(); break;
        case 3: _t->open_session_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->open_recent_session_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->reopen_last_session_requested(); break;
        case 6: _t->delete_session_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (StartPageWidget::*)();
            if (_t _q_method = &StartPageWidget::open_log_file_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (StartPageWidget::*)(const QString & );
            if (_t _q_method = &StartPageWidget::open_recent_file_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (StartPageWidget::*)();
            if (_t _q_method = &StartPageWidget::clear_recent_files_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (StartPageWidget::*)(const QString & );
            if (_t _q_method = &StartPageWidget::open_session_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (StartPageWidget::*)(const QString & );
            if (_t _q_method = &StartPageWidget::open_recent_session_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (StartPageWidget::*)();
            if (_t _q_method = &StartPageWidget::reopen_last_session_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (StartPageWidget::*)(const QString & );
            if (_t _q_method = &StartPageWidget::delete_session_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
    }
}

const QMetaObject *StartPageWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StartPageWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_StartPageWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int StartPageWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void StartPageWidget::open_log_file_requested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void StartPageWidget::open_recent_file_requested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void StartPageWidget::clear_recent_files_requested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void StartPageWidget::open_session_requested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void StartPageWidget::open_recent_session_requested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void StartPageWidget::reopen_last_session_requested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void StartPageWidget::delete_session_requested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
