/****************************************************************************
** Meta object code from reading C++ file 'LogIngestController.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Controllers/LogIngestController.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LogIngestController.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LogIngestController_t {
    uint offsetsAndSizes[28];
    char stringdata0[20];
    char stringdata1[19];
    char stringdata2[1];
    char stringdata3[8];
    char stringdata4[10];
    char stringdata5[16];
    char stringdata6[6];
    char stringdata7[9];
    char stringdata8[11];
    char stringdata9[12];
    char stringdata10[6];
    char stringdata11[8];
    char stringdata12[9];
    char stringdata13[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LogIngestController_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LogIngestController_t qt_meta_stringdata_LogIngestController = {
    {
        QT_MOC_LITERAL(0, 19),  // "LogIngestController"
        QT_MOC_LITERAL(20, 18),  // "entry_batch_parsed"
        QT_MOC_LITERAL(39, 0),  // ""
        QT_MOC_LITERAL(40, 7),  // "view_id"
        QT_MOC_LITERAL(48, 9),  // "file_path"
        QT_MOC_LITERAL(58, 15),  // "QList<LogEntry>"
        QT_MOC_LITERAL(74, 5),  // "batch"
        QT_MOC_LITERAL(80, 8),  // "progress"
        QT_MOC_LITERAL(89, 10),  // "bytes_read"
        QT_MOC_LITERAL(100, 11),  // "total_bytes"
        QT_MOC_LITERAL(112, 5),  // "error"
        QT_MOC_LITERAL(118, 7),  // "message"
        QT_MOC_LITERAL(126, 8),  // "finished"
        QT_MOC_LITERAL(135, 4)   // "idle"
    },
    "LogIngestController",
    "entry_batch_parsed",
    "",
    "view_id",
    "file_path",
    "QList<LogEntry>",
    "batch",
    "progress",
    "bytes_read",
    "total_bytes",
    "error",
    "message",
    "finished",
    "idle"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LogIngestController[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    3,   44,    2, 0x06,    1 /* Public */,
       7,    4,   51,    2, 0x06,    5 /* Public */,
      10,    3,   60,    2, 0x06,   10 /* Public */,
      12,    2,   67,    2, 0x06,   14 /* Public */,
      13,    0,   72,    2, 0x06,   17 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QUuid, QMetaType::QString, 0x80000000 | 5,    3,    4,    6,
    QMetaType::Void, QMetaType::QUuid, QMetaType::QString, QMetaType::LongLong, QMetaType::LongLong,    3,    4,    8,    9,
    QMetaType::Void, QMetaType::QUuid, QMetaType::QString, QMetaType::QString,    3,    4,   11,
    QMetaType::Void, QMetaType::QUuid, QMetaType::QString,    3,    4,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject LogIngestController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_LogIngestController.offsetsAndSizes,
    qt_meta_data_LogIngestController,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LogIngestController_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LogIngestController, std::true_type>,
        // method 'entry_batch_parsed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<LogEntry> &, std::false_type>,
        // method 'progress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'error'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'finished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'idle'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void LogIngestController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LogIngestController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->entry_batch_parsed((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QList<LogEntry>>>(_a[3]))); break;
        case 1: _t->progress((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[4]))); break;
        case 2: _t->error((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 3: _t->finished((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: _t->idle(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LogIngestController::*)(const QUuid & , const QString & , const QVector<LogEntry> & );
            if (_t _q_method = &LogIngestController::entry_batch_parsed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LogIngestController::*)(const QUuid & , const QString & , qint64 , qint64 );
            if (_t _q_method = &LogIngestController::progress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LogIngestController::*)(const QUuid & , const QString & , const QString & );
            if (_t _q_method = &LogIngestController::error; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LogIngestController::*)(const QUuid & , const QString & );
            if (_t _q_method = &LogIngestController::finished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LogIngestController::*)();
            if (_t _q_method = &LogIngestController::idle; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject *LogIngestController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LogIngestController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LogIngestController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int LogIngestController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void LogIngestController::entry_batch_parsed(const QUuid & _t1, const QString & _t2, const QVector<LogEntry> & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LogIngestController::progress(const QUuid & _t1, const QString & _t2, qint64 _t3, qint64 _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void LogIngestController::error(const QUuid & _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void LogIngestController::finished(const QUuid & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void LogIngestController::idle()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
