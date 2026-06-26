/****************************************************************************
** Meta object code from reading C++ file 'SearchBarWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Views/App/SearchBarWidget.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SearchBarWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_SearchBarWidget_t {
    uint offsetsAndSizes[30];
    char stringdata0[16];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[6];
    char stringdata5[6];
    char stringdata6[20];
    char stringdata7[21];
    char stringdata8[14];
    char stringdata9[8];
    char stringdata10[20];
    char stringdata11[17];
    char stringdata12[23];
    char stringdata13[14];
    char stringdata14[21];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_SearchBarWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_SearchBarWidget_t qt_meta_stringdata_SearchBarWidget = {
    {
        QT_MOC_LITERAL(0, 15),  // "SearchBarWidget"
        QT_MOC_LITERAL(16, 16),  // "search_requested"
        QT_MOC_LITERAL(33, 0),  // ""
        QT_MOC_LITERAL(34, 4),  // "text"
        QT_MOC_LITERAL(39, 5),  // "field"
        QT_MOC_LITERAL(45, 5),  // "regex"
        QT_MOC_LITERAL(51, 19),  // "search_text_changed"
        QT_MOC_LITERAL(71, 20),  // "search_field_changed"
        QT_MOC_LITERAL(92, 13),  // "regex_toggled"
        QT_MOC_LITERAL(106, 7),  // "enabled"
        QT_MOC_LITERAL(114, 19),  // "live_search_toggled"
        QT_MOC_LITERAL(134, 16),  // "clear_icon_color"
        QT_MOC_LITERAL(151, 22),  // "clear_icon_color_hover"
        QT_MOC_LITERAL(174, 13),  // "clear_icon_px"
        QT_MOC_LITERAL(188, 20)   // "clear_button_padding"
    },
    "SearchBarWidget",
    "search_requested",
    "",
    "text",
    "field",
    "regex",
    "search_text_changed",
    "search_field_changed",
    "regex_toggled",
    "enabled",
    "live_search_toggled",
    "clear_icon_color",
    "clear_icon_color_hover",
    "clear_icon_px",
    "clear_button_padding"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_SearchBarWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       4,   63, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    3,   44,    2, 0x06,    5 /* Public */,
       6,    1,   51,    2, 0x06,    9 /* Public */,
       7,    1,   54,    2, 0x06,   11 /* Public */,
       8,    1,   57,    2, 0x06,   13 /* Public */,
      10,    1,   60,    2, 0x06,   15 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::Bool,    3,    4,    5,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void, QMetaType::Bool,    9,
    QMetaType::Void, QMetaType::Bool,    9,

 // properties: name, type, flags
      11, QMetaType::QColor, 0x00015003, uint(-1), 0,
      12, QMetaType::QColor, 0x00015003, uint(-1), 0,
      13, QMetaType::Int, 0x00015003, uint(-1), 0,
      14, QMetaType::Int, 0x00015003, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject SearchBarWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_SearchBarWidget.offsetsAndSizes,
    qt_meta_data_SearchBarWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_SearchBarWidget_t,
        // property 'clear_icon_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'clear_icon_color_hover'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'clear_icon_px'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'clear_button_padding'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SearchBarWidget, std::true_type>,
        // method 'search_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'search_text_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'search_field_changed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'regex_toggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'live_search_toggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>
    >,
    nullptr
} };

void SearchBarWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SearchBarWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->search_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3]))); break;
        case 1: _t->search_text_changed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->search_field_changed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->regex_toggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->live_search_toggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SearchBarWidget::*)(const QString & , const QString & , bool );
            if (_t _q_method = &SearchBarWidget::search_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SearchBarWidget::*)(const QString & );
            if (_t _q_method = &SearchBarWidget::search_text_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SearchBarWidget::*)(const QString & );
            if (_t _q_method = &SearchBarWidget::search_field_changed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SearchBarWidget::*)(bool );
            if (_t _q_method = &SearchBarWidget::regex_toggled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (SearchBarWidget::*)(bool );
            if (_t _q_method = &SearchBarWidget::live_search_toggled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<SearchBarWidget *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QColor*>(_v) = _t->get_clear_icon_color(); break;
        case 1: *reinterpret_cast< QColor*>(_v) = _t->get_clear_icon_color_hover(); break;
        case 2: *reinterpret_cast< int*>(_v) = _t->get_clear_icon_px(); break;
        case 3: *reinterpret_cast< int*>(_v) = _t->get_clear_button_padding(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<SearchBarWidget *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->set_clear_icon_color(*reinterpret_cast< QColor*>(_v)); break;
        case 1: _t->set_clear_icon_color_hover(*reinterpret_cast< QColor*>(_v)); break;
        case 2: _t->set_clear_icon_px(*reinterpret_cast< int*>(_v)); break;
        case 3: _t->set_clear_button_padding(*reinterpret_cast< int*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *SearchBarWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SearchBarWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SearchBarWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SearchBarWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void SearchBarWidget::search_requested(const QString & _t1, const QString & _t2, bool _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SearchBarWidget::search_text_changed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SearchBarWidget::search_field_changed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void SearchBarWidget::regex_toggled(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void SearchBarWidget::live_search_toggled(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
