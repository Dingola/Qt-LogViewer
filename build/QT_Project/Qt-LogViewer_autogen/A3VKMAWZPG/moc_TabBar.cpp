/****************************************************************************
** Meta object code from reading C++ file 'TabBar.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Views/Shared/TabBar.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TabBar.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_TabBar_t {
    uint offsetsAndSizes[30];
    char stringdata0[7];
    char stringdata1[27];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[7];
    char stringdata5[5];
    char stringdata6[20];
    char stringdata7[27];
    char stringdata8[29];
    char stringdata9[30];
    char stringdata10[28];
    char stringdata11[17];
    char stringdata12[23];
    char stringdata13[14];
    char stringdata14[28];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_TabBar_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_TabBar_t qt_meta_stringdata_TabBar = {
    {
        QT_MOC_LITERAL(0, 6),  // "TabBar"
        QT_MOC_LITERAL(7, 26),  // "about_to_show_context_menu"
        QT_MOC_LITERAL(34, 0),  // ""
        QT_MOC_LITERAL(35, 9),  // "tab_index"
        QT_MOC_LITERAL(45, 6),  // "QMenu*"
        QT_MOC_LITERAL(52, 4),  // "menu"
        QT_MOC_LITERAL(57, 19),  // "close_tab_requested"
        QT_MOC_LITERAL(77, 26),  // "close_other_tabs_requested"
        QT_MOC_LITERAL(104, 28),  // "close_tabs_to_left_requested"
        QT_MOC_LITERAL(133, 29),  // "close_tabs_to_right_requested"
        QT_MOC_LITERAL(163, 27),  // "handle_close_button_clicked"
        QT_MOC_LITERAL(191, 16),  // "close_icon_color"
        QT_MOC_LITERAL(208, 22),  // "close_icon_color_hover"
        QT_MOC_LITERAL(231, 13),  // "close_icon_px"
        QT_MOC_LITERAL(245, 27)   // "enable_default_context_menu"
    },
    "TabBar",
    "about_to_show_context_menu",
    "",
    "tab_index",
    "QMenu*",
    "menu",
    "close_tab_requested",
    "close_other_tabs_requested",
    "close_tabs_to_left_requested",
    "close_tabs_to_right_requested",
    "handle_close_button_clicked",
    "close_icon_color",
    "close_icon_color_hover",
    "close_icon_px",
    "enable_default_context_menu"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_TabBar[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       4,   68, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   50,    2, 0x06,    5 /* Public */,
       6,    1,   55,    2, 0x06,    8 /* Public */,
       7,    1,   58,    2, 0x06,   10 /* Public */,
       8,    1,   61,    2, 0x06,   12 /* Public */,
       9,    1,   64,    2, 0x06,   14 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      10,    0,   67,    2, 0x08,   16 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 4,    3,    5,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,

 // slots: parameters
    QMetaType::Void,

 // properties: name, type, flags
      11, QMetaType::QColor, 0x00015003, uint(-1), 0,
      12, QMetaType::QColor, 0x00015003, uint(-1), 0,
      13, QMetaType::Int, 0x00015003, uint(-1), 0,
      14, QMetaType::Bool, 0x00015003, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject TabBar::staticMetaObject = { {
    QMetaObject::SuperData::link<QTabBar::staticMetaObject>(),
    qt_meta_stringdata_TabBar.offsetsAndSizes,
    qt_meta_data_TabBar,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_TabBar_t,
        // property 'close_icon_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'close_icon_color_hover'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'close_icon_px'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'enable_default_context_menu'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TabBar, std::true_type>,
        // method 'about_to_show_context_menu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<QMenu *, std::false_type>,
        // method 'close_tab_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'close_other_tabs_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'close_tabs_to_left_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'close_tabs_to_right_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'handle_close_button_clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void TabBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TabBar *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->about_to_show_context_menu((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QMenu*>>(_a[2]))); break;
        case 1: _t->close_tab_requested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->close_other_tabs_requested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->close_tabs_to_left_requested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->close_tabs_to_right_requested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->handle_close_button_clicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TabBar::*)(int , QMenu * );
            if (_t _q_method = &TabBar::about_to_show_context_menu; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TabBar::*)(int );
            if (_t _q_method = &TabBar::close_tab_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TabBar::*)(int );
            if (_t _q_method = &TabBar::close_other_tabs_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TabBar::*)(int );
            if (_t _q_method = &TabBar::close_tabs_to_left_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (TabBar::*)(int );
            if (_t _q_method = &TabBar::close_tabs_to_right_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<TabBar *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QColor*>(_v) = _t->get_close_icon_color(); break;
        case 1: *reinterpret_cast< QColor*>(_v) = _t->get_close_icon_color_hover(); break;
        case 2: *reinterpret_cast< int*>(_v) = _t->get_close_icon_px(); break;
        case 3: *reinterpret_cast< bool*>(_v) = _t->get_enable_default_context_menu(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<TabBar *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->set_close_icon_color(*reinterpret_cast< QColor*>(_v)); break;
        case 1: _t->set_close_icon_color_hover(*reinterpret_cast< QColor*>(_v)); break;
        case 2: _t->set_close_icon_px(*reinterpret_cast< int*>(_v)); break;
        case 3: _t->set_enable_default_context_menu(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *TabBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TabBar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TabBar.stringdata0))
        return static_cast<void*>(this);
    return QTabBar::qt_metacast(_clname);
}

int TabBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTabBar::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void TabBar::about_to_show_context_menu(int _t1, QMenu * _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void TabBar::close_tab_requested(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void TabBar::close_other_tabs_requested(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void TabBar::close_tabs_to_left_requested(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void TabBar::close_tabs_to_right_requested(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
