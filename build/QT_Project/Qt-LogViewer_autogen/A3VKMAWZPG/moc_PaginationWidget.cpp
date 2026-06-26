/****************************************************************************
** Meta object code from reading C++ file 'PaginationWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Views/Shared/PaginationWidget.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PaginationWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_PaginationWidget_t {
    uint offsetsAndSizes[24];
    char stringdata0[17];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[23];
    char stringdata5[15];
    char stringdata6[14];
    char stringdata7[14];
    char stringdata8[20];
    char stringdata9[13];
    char stringdata10[22];
    char stringdata11[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_PaginationWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_PaginationWidget_t qt_meta_stringdata_PaginationWidget = {
    {
        QT_MOC_LITERAL(0, 16),  // "PaginationWidget"
        QT_MOC_LITERAL(17, 12),  // "page_changed"
        QT_MOC_LITERAL(30, 0),  // ""
        QT_MOC_LITERAL(31, 4),  // "page"
        QT_MOC_LITERAL(36, 22),  // "items_per_page_changed"
        QT_MOC_LITERAL(59, 14),  // "items_per_page"
        QT_MOC_LITERAL(74, 13),  // "onPrevClicked"
        QT_MOC_LITERAL(88, 13),  // "onNextClicked"
        QT_MOC_LITERAL(102, 19),  // "onPageButtonClicked"
        QT_MOC_LITERAL(122, 12),  // "onJumpToPage"
        QT_MOC_LITERAL(135, 21),  // "onItemsPerPageChanged"
        QT_MOC_LITERAL(157, 5)   // "index"
    },
    "PaginationWidget",
    "page_changed",
    "",
    "page",
    "items_per_page_changed",
    "items_per_page",
    "onPrevClicked",
    "onNextClicked",
    "onPageButtonClicked",
    "onJumpToPage",
    "onItemsPerPageChanged",
    "index"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_PaginationWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   56,    2, 0x06,    1 /* Public */,
       4,    1,   59,    2, 0x06,    3 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       6,    0,   62,    2, 0x08,    5 /* Private */,
       7,    0,   63,    2, 0x08,    6 /* Private */,
       8,    0,   64,    2, 0x08,    7 /* Private */,
       9,    0,   65,    2, 0x08,    8 /* Private */,
      10,    1,   66,    2, 0x08,    9 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    5,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   11,

       0        // eod
};

Q_CONSTINIT const QMetaObject PaginationWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_PaginationWidget.offsetsAndSizes,
    qt_meta_data_PaginationWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_PaginationWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PaginationWidget, std::true_type>,
        // method 'page_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'items_per_page_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onPrevClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onNextClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPageButtonClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onJumpToPage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onItemsPerPageChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void PaginationWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PaginationWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->page_changed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->items_per_page_changed((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->onPrevClicked(); break;
        case 3: _t->onNextClicked(); break;
        case 4: _t->onPageButtonClicked(); break;
        case 5: _t->onJumpToPage(); break;
        case 6: _t->onItemsPerPageChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PaginationWidget::*)(int );
            if (_t _q_method = &PaginationWidget::page_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PaginationWidget::*)(int );
            if (_t _q_method = &PaginationWidget::items_per_page_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject *PaginationWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PaginationWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PaginationWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int PaginationWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void PaginationWidget::page_changed(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PaginationWidget::items_per_page_changed(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
