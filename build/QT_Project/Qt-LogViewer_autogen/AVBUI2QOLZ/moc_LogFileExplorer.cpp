/****************************************************************************
** Meta object code from reading C++ file 'LogFileExplorer.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Views/App/LogFileExplorer.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LogFileExplorer.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LogFileExplorer_t {
    uint offsetsAndSizes[28];
    char stringdata0[16];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[12];
    char stringdata4[14];
    char stringdata5[20];
    char stringdata6[30];
    char stringdata7[22];
    char stringdata8[17];
    char stringdata9[11];
    char stringdata10[25];
    char stringdata11[9];
    char stringdata12[24];
    char stringdata13[25];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LogFileExplorer_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LogFileExplorer_t qt_meta_stringdata_LogFileExplorer = {
    {
        QT_MOC_LITERAL(0, 15),  // "LogFileExplorer"
        QT_MOC_LITERAL(16, 13),  // "file_selected"
        QT_MOC_LITERAL(30, 0),  // ""
        QT_MOC_LITERAL(31, 11),  // "LogFileInfo"
        QT_MOC_LITERAL(43, 13),  // "log_file_info"
        QT_MOC_LITERAL(57, 19),  // "open_file_requested"
        QT_MOC_LITERAL(77, 29),  // "add_to_current_view_requested"
        QT_MOC_LITERAL(107, 21),  // "remove_file_requested"
        QT_MOC_LITERAL(129, 16),  // "session_selected"
        QT_MOC_LITERAL(146, 10),  // "session_id"
        QT_MOC_LITERAL(157, 24),  // "rename_session_requested"
        QT_MOC_LITERAL(182, 8),  // "new_name"
        QT_MOC_LITERAL(191, 23),  // "close_session_requested"
        QT_MOC_LITERAL(215, 24)   // "delete_session_requested"
    },
    "LogFileExplorer",
    "file_selected",
    "",
    "LogFileInfo",
    "log_file_info",
    "open_file_requested",
    "add_to_current_view_requested",
    "remove_file_requested",
    "session_selected",
    "session_id",
    "rename_session_requested",
    "new_name",
    "close_session_requested",
    "delete_session_requested"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LogFileExplorer[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   62,    2, 0x06,    1 /* Public */,
       5,    1,   65,    2, 0x06,    3 /* Public */,
       6,    1,   68,    2, 0x06,    5 /* Public */,
       7,    1,   71,    2, 0x06,    7 /* Public */,
       8,    1,   74,    2, 0x06,    9 /* Public */,
      10,    2,   77,    2, 0x06,   11 /* Public */,
      12,    1,   82,    2, 0x06,   14 /* Public */,
      13,    1,   85,    2, 0x06,   16 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    9,   11,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, QMetaType::QString,    9,

       0        // eod
};

Q_CONSTINIT const QMetaObject LogFileExplorer::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_LogFileExplorer.offsetsAndSizes,
    qt_meta_data_LogFileExplorer,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LogFileExplorer_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LogFileExplorer, std::true_type>,
        // method 'file_selected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const LogFileInfo &, std::false_type>,
        // method 'open_file_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const LogFileInfo &, std::false_type>,
        // method 'add_to_current_view_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const LogFileInfo &, std::false_type>,
        // method 'remove_file_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const LogFileInfo &, std::false_type>,
        // method 'session_selected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'rename_session_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'close_session_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'delete_session_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void LogFileExplorer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LogFileExplorer *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->file_selected((*reinterpret_cast< std::add_pointer_t<LogFileInfo>>(_a[1]))); break;
        case 1: _t->open_file_requested((*reinterpret_cast< std::add_pointer_t<LogFileInfo>>(_a[1]))); break;
        case 2: _t->add_to_current_view_requested((*reinterpret_cast< std::add_pointer_t<LogFileInfo>>(_a[1]))); break;
        case 3: _t->remove_file_requested((*reinterpret_cast< std::add_pointer_t<LogFileInfo>>(_a[1]))); break;
        case 4: _t->session_selected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->rename_session_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 6: _t->close_session_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->delete_session_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LogFileExplorer::*)(const LogFileInfo & );
            if (_t _q_method = &LogFileExplorer::file_selected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LogFileExplorer::*)(const LogFileInfo & );
            if (_t _q_method = &LogFileExplorer::open_file_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LogFileExplorer::*)(const LogFileInfo & );
            if (_t _q_method = &LogFileExplorer::add_to_current_view_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LogFileExplorer::*)(const LogFileInfo & );
            if (_t _q_method = &LogFileExplorer::remove_file_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LogFileExplorer::*)(const QString & );
            if (_t _q_method = &LogFileExplorer::session_selected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (LogFileExplorer::*)(const QString & , const QString & );
            if (_t _q_method = &LogFileExplorer::rename_session_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (LogFileExplorer::*)(const QString & );
            if (_t _q_method = &LogFileExplorer::close_session_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (LogFileExplorer::*)(const QString & );
            if (_t _q_method = &LogFileExplorer::delete_session_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
    }
}

const QMetaObject *LogFileExplorer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LogFileExplorer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LogFileExplorer.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int LogFileExplorer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void LogFileExplorer::file_selected(const LogFileInfo & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LogFileExplorer::open_file_requested(const LogFileInfo & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void LogFileExplorer::add_to_current_view_requested(const LogFileInfo & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void LogFileExplorer::remove_file_requested(const LogFileInfo & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void LogFileExplorer::session_selected(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void LogFileExplorer::rename_session_requested(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void LogFileExplorer::close_session_requested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void LogFileExplorer::delete_session_requested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
