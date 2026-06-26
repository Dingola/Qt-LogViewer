/****************************************************************************
** Meta object code from reading C++ file 'LogViewerController.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Controllers/LogViewerController.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LogViewerController.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LogViewerController_t {
    uint offsetsAndSizes[34];
    char stringdata0[20];
    char stringdata1[24];
    char stringdata2[1];
    char stringdata3[8];
    char stringdata4[13];
    char stringdata5[17];
    char stringdata6[11];
    char stringdata7[12];
    char stringdata8[17];
    char stringdata9[10];
    char stringdata10[14];
    char stringdata11[8];
    char stringdata12[24];
    char stringdata13[11];
    char stringdata14[16];
    char stringdata15[12];
    char stringdata16[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LogViewerController_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LogViewerController_t qt_meta_stringdata_LogViewerController = {
    {
        QT_MOC_LITERAL(0, 19),  // "LogViewerController"
        QT_MOC_LITERAL(20, 23),  // "current_view_id_changed"
        QT_MOC_LITERAL(44, 0),  // ""
        QT_MOC_LITERAL(45, 7),  // "view_id"
        QT_MOC_LITERAL(53, 12),  // "view_removed"
        QT_MOC_LITERAL(66, 16),  // "loading_progress"
        QT_MOC_LITERAL(83, 10),  // "bytes_read"
        QT_MOC_LITERAL(94, 11),  // "total_bytes"
        QT_MOC_LITERAL(106, 16),  // "loading_finished"
        QT_MOC_LITERAL(123, 9),  // "file_path"
        QT_MOC_LITERAL(133, 13),  // "loading_error"
        QT_MOC_LITERAL(147, 7),  // "message"
        QT_MOC_LITERAL(155, 23),  // "view_file_paths_changed"
        QT_MOC_LITERAL(179, 10),  // "file_paths"
        QT_MOC_LITERAL(190, 15),  // "remove_log_file"
        QT_MOC_LITERAL(206, 11),  // "LogFileInfo"
        QT_MOC_LITERAL(218, 4)   // "file"
    },
    "LogViewerController",
    "current_view_id_changed",
    "",
    "view_id",
    "view_removed",
    "loading_progress",
    "bytes_read",
    "total_bytes",
    "loading_finished",
    "file_path",
    "loading_error",
    "message",
    "view_file_paths_changed",
    "file_paths",
    "remove_log_file",
    "LogFileInfo",
    "file"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LogViewerController[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   62,    2, 0x06,    1 /* Public */,
       4,    1,   65,    2, 0x06,    3 /* Public */,
       5,    3,   68,    2, 0x06,    5 /* Public */,
       8,    2,   75,    2, 0x06,    9 /* Public */,
      10,    3,   80,    2, 0x06,   12 /* Public */,
      12,    2,   87,    2, 0x06,   16 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      14,    1,   92,    2, 0x0a,   19 /* Public */,
      14,    2,   95,    2, 0x0a,   21 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QUuid,    3,
    QMetaType::Void, QMetaType::QUuid,    3,
    QMetaType::Void, QMetaType::QUuid, QMetaType::LongLong, QMetaType::LongLong,    3,    6,    7,
    QMetaType::Void, QMetaType::QUuid, QMetaType::QString,    3,    9,
    QMetaType::Void, QMetaType::QUuid, QMetaType::QString, QMetaType::QString,    3,    9,   11,
    QMetaType::Void, QMetaType::QUuid, QMetaType::QStringList,    3,   13,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void, QMetaType::QUuid, QMetaType::QString,    3,    9,

       0        // eod
};

Q_CONSTINIT const QMetaObject LogViewerController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_LogViewerController.offsetsAndSizes,
    qt_meta_data_LogViewerController,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LogViewerController_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LogViewerController, std::true_type>,
        // method 'current_view_id_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        // method 'view_removed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        // method 'loading_progress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'loading_finished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'loading_error'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'view_file_paths_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<QString> &, std::false_type>,
        // method 'remove_log_file'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const LogFileInfo &, std::false_type>,
        // method 'remove_log_file'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void LogViewerController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LogViewerController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->current_view_id_changed((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        case 1: _t->view_removed((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        case 2: _t->loading_progress((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[3]))); break;
        case 3: _t->loading_finished((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: _t->loading_error((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 5: _t->view_file_paths_changed((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<QString>>>(_a[2]))); break;
        case 6: _t->remove_log_file((*reinterpret_cast< std::add_pointer_t<LogFileInfo>>(_a[1]))); break;
        case 7: _t->remove_log_file((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< LogFileInfo >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LogViewerController::*)(const QUuid & );
            if (_t _q_method = &LogViewerController::current_view_id_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LogViewerController::*)(const QUuid & );
            if (_t _q_method = &LogViewerController::view_removed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LogViewerController::*)(const QUuid & , qint64 , qint64 );
            if (_t _q_method = &LogViewerController::loading_progress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LogViewerController::*)(const QUuid & , const QString & );
            if (_t _q_method = &LogViewerController::loading_finished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LogViewerController::*)(const QUuid & , const QString & , const QString & );
            if (_t _q_method = &LogViewerController::loading_error; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (LogViewerController::*)(const QUuid & , const QVector<QString> & );
            if (_t _q_method = &LogViewerController::view_file_paths_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject *LogViewerController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LogViewerController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LogViewerController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int LogViewerController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void LogViewerController::current_view_id_changed(const QUuid & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LogViewerController::view_removed(const QUuid & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void LogViewerController::loading_progress(const QUuid & _t1, qint64 _t2, qint64 _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void LogViewerController::loading_finished(const QUuid & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void LogViewerController::loading_error(const QUuid & _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void LogViewerController::view_file_paths_changed(const QUuid & _t1, const QVector<QString> & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
