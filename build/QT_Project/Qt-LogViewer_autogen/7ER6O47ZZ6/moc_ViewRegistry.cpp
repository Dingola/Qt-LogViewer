/****************************************************************************
** Meta object code from reading C++ file 'ViewRegistry.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Controllers/ViewRegistry.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ViewRegistry.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_ViewRegistry_t {
    uint offsetsAndSizes[14];
    char stringdata0[13];
    char stringdata1[24];
    char stringdata2[1];
    char stringdata3[8];
    char stringdata4[13];
    char stringdata5[24];
    char stringdata6[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_ViewRegistry_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_ViewRegistry_t qt_meta_stringdata_ViewRegistry = {
    {
        QT_MOC_LITERAL(0, 12),  // "ViewRegistry"
        QT_MOC_LITERAL(13, 23),  // "current_view_id_changed"
        QT_MOC_LITERAL(37, 0),  // ""
        QT_MOC_LITERAL(38, 7),  // "view_id"
        QT_MOC_LITERAL(46, 12),  // "view_removed"
        QT_MOC_LITERAL(59, 23),  // "view_file_paths_changed"
        QT_MOC_LITERAL(83, 10)   // "file_paths"
    },
    "ViewRegistry",
    "current_view_id_changed",
    "",
    "view_id",
    "view_removed",
    "view_file_paths_changed",
    "file_paths"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_ViewRegistry[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   32,    2, 0x06,    1 /* Public */,
       4,    1,   35,    2, 0x06,    3 /* Public */,
       5,    2,   38,    2, 0x06,    5 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QUuid,    3,
    QMetaType::Void, QMetaType::QUuid,    3,
    QMetaType::Void, QMetaType::QUuid, QMetaType::QStringList,    3,    6,

       0        // eod
};

Q_CONSTINIT const QMetaObject ViewRegistry::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ViewRegistry.offsetsAndSizes,
    qt_meta_data_ViewRegistry,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_ViewRegistry_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ViewRegistry, std::true_type>,
        // method 'current_view_id_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        // method 'view_removed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        // method 'view_file_paths_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<QString> &, std::false_type>
    >,
    nullptr
} };

void ViewRegistry::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ViewRegistry *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->current_view_id_changed((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        case 1: _t->view_removed((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        case 2: _t->view_file_paths_changed((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QList<QString>>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ViewRegistry::*)(const QUuid & );
            if (_t _q_method = &ViewRegistry::current_view_id_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ViewRegistry::*)(const QUuid & );
            if (_t _q_method = &ViewRegistry::view_removed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ViewRegistry::*)(const QUuid & , const QVector<QString> & );
            if (_t _q_method = &ViewRegistry::view_file_paths_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *ViewRegistry::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ViewRegistry::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ViewRegistry.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ViewRegistry::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void ViewRegistry::current_view_id_changed(const QUuid & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ViewRegistry::view_removed(const QUuid & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void ViewRegistry::view_file_paths_changed(const QUuid & _t1, const QVector<QString> & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
