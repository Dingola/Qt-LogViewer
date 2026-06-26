/****************************************************************************
** Meta object code from reading C++ file 'LogViewWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Views/App/LogViewWidget.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QSet>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LogViewWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LogViewWidget_t {
    uint offsetsAndSizes[34];
    char stringdata0[14];
    char stringdata1[19];
    char stringdata2[1];
    char stringdata3[9];
    char stringdata4[25];
    char stringdata5[14];
    char stringdata6[7];
    char stringdata7[16];
    char stringdata8[8];
    char stringdata9[20];
    char stringdata10[12];
    char stringdata11[8];
    char stringdata12[9];
    char stringdata13[25];
    char stringdata14[10];
    char stringdata15[28];
    char stringdata16[22];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LogViewWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LogViewWidget_t qt_meta_stringdata_LogViewWidget = {
    {
        QT_MOC_LITERAL(0, 13),  // "LogViewWidget"
        QT_MOC_LITERAL(14, 18),  // "app_filter_changed"
        QT_MOC_LITERAL(33, 0),  // ""
        QT_MOC_LITERAL(34, 8),  // "app_name"
        QT_MOC_LITERAL(43, 24),  // "log_level_filter_changed"
        QT_MOC_LITERAL(68, 13),  // "QSet<QString>"
        QT_MOC_LITERAL(82, 6),  // "levels"
        QT_MOC_LITERAL(89, 15),  // "view_id_changed"
        QT_MOC_LITERAL(105, 7),  // "view_id"
        QT_MOC_LITERAL(113, 19),  // "current_row_changed"
        QT_MOC_LITERAL(133, 11),  // "QModelIndex"
        QT_MOC_LITERAL(145, 7),  // "current"
        QT_MOC_LITERAL(153, 8),  // "previous"
        QT_MOC_LITERAL(162, 24),  // "show_only_file_requested"
        QT_MOC_LITERAL(187, 9),  // "file_path"
        QT_MOC_LITERAL(197, 27),  // "toggle_visibility_requested"
        QT_MOC_LITERAL(225, 21)   // "remove_file_requested"
    },
    "LogViewWidget",
    "app_filter_changed",
    "",
    "app_name",
    "log_level_filter_changed",
    "QSet<QString>",
    "levels",
    "view_id_changed",
    "view_id",
    "current_row_changed",
    "QModelIndex",
    "current",
    "previous",
    "show_only_file_requested",
    "file_path",
    "toggle_visibility_requested",
    "remove_file_requested"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LogViewWidget[] = {

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
       1,    1,   56,    2, 0x06,    1 /* Public */,
       4,    1,   59,    2, 0x06,    3 /* Public */,
       7,    1,   62,    2, 0x06,    5 /* Public */,
       9,    2,   65,    2, 0x06,    7 /* Public */,
      13,    1,   70,    2, 0x06,   10 /* Public */,
      15,    1,   73,    2, 0x06,   12 /* Public */,
      16,    1,   76,    2, 0x06,   14 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, QMetaType::QUuid,    8,
    QMetaType::Void, 0x80000000 | 10, 0x80000000 | 10,   11,   12,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void, QMetaType::QString,   14,

       0        // eod
};

Q_CONSTINIT const QMetaObject LogViewWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_LogViewWidget.offsetsAndSizes,
    qt_meta_data_LogViewWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LogViewWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LogViewWidget, std::true_type>,
        // method 'app_filter_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'log_level_filter_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QSet<QString> &, std::false_type>,
        // method 'view_id_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        // method 'current_row_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QModelIndex &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QModelIndex &, std::false_type>,
        // method 'show_only_file_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'toggle_visibility_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'remove_file_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void LogViewWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LogViewWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->app_filter_changed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->log_level_filter_changed((*reinterpret_cast< std::add_pointer_t<QSet<QString>>>(_a[1]))); break;
        case 2: _t->view_id_changed((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        case 3: _t->current_row_changed((*reinterpret_cast< std::add_pointer_t<QModelIndex>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QModelIndex>>(_a[2]))); break;
        case 4: _t->show_only_file_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->toggle_visibility_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->remove_file_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QSet<QString> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LogViewWidget::*)(const QString & );
            if (_t _q_method = &LogViewWidget::app_filter_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LogViewWidget::*)(const QSet<QString> & );
            if (_t _q_method = &LogViewWidget::log_level_filter_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LogViewWidget::*)(const QUuid & );
            if (_t _q_method = &LogViewWidget::view_id_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LogViewWidget::*)(const QModelIndex & , const QModelIndex & );
            if (_t _q_method = &LogViewWidget::current_row_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LogViewWidget::*)(const QString & );
            if (_t _q_method = &LogViewWidget::show_only_file_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (LogViewWidget::*)(const QString & );
            if (_t _q_method = &LogViewWidget::toggle_visibility_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (LogViewWidget::*)(const QString & );
            if (_t _q_method = &LogViewWidget::remove_file_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
    }
}

const QMetaObject *LogViewWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LogViewWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LogViewWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int LogViewWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void LogViewWidget::app_filter_changed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LogViewWidget::log_level_filter_changed(const QSet<QString> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void LogViewWidget::view_id_changed(const QUuid & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void LogViewWidget::current_row_changed(const QModelIndex & _t1, const QModelIndex & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void LogViewWidget::show_only_file_requested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void LogViewWidget::toggle_visibility_requested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void LogViewWidget::remove_file_requested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
