/****************************************************************************
** Meta object code from reading C++ file 'FilesInViewMenuItemWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Views/App/FilesInViewMenuItemWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FilesInViewMenuItemWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_FilesInViewMenuItemWidget_t {
    uint offsetsAndSizes[32];
    char stringdata0[26];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[28];
    char stringdata5[17];
    char stringdata6[11];
    char stringdata7[17];
    char stringdata8[15];
    char stringdata9[19];
    char stringdata10[17];
    char stringdata11[8];
    char stringdata12[14];
    char stringdata13[18];
    char stringdata14[16];
    char stringdata15[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_FilesInViewMenuItemWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_FilesInViewMenuItemWidget_t qt_meta_stringdata_FilesInViewMenuItemWidget = {
    {
        QT_MOC_LITERAL(0, 25),  // "FilesInViewMenuItemWidget"
        QT_MOC_LITERAL(26, 19),  // "show_only_requested"
        QT_MOC_LITERAL(46, 0),  // ""
        QT_MOC_LITERAL(47, 9),  // "file_path"
        QT_MOC_LITERAL(57, 27),  // "toggle_visibility_requested"
        QT_MOC_LITERAL(85, 16),  // "remove_requested"
        QT_MOC_LITERAL(102, 10),  // "icon_color"
        QT_MOC_LITERAL(113, 16),  // "icon_color_hover"
        QT_MOC_LITERAL(130, 14),  // "icon_color_eye"
        QT_MOC_LITERAL(145, 18),  // "icon_color_eye_off"
        QT_MOC_LITERAL(164, 16),  // "icon_color_trash"
        QT_MOC_LITERAL(181, 7),  // "icon_px"
        QT_MOC_LITERAL(189, 13),  // "icon_eye_path"
        QT_MOC_LITERAL(203, 17),  // "icon_eye_off_path"
        QT_MOC_LITERAL(221, 15),  // "icon_trash_path"
        QT_MOC_LITERAL(237, 17)   // "label_reserved_px"
    },
    "FilesInViewMenuItemWidget",
    "show_only_requested",
    "",
    "file_path",
    "toggle_visibility_requested",
    "remove_requested",
    "icon_color",
    "icon_color_hover",
    "icon_color_eye",
    "icon_color_eye_off",
    "icon_color_trash",
    "icon_px",
    "icon_eye_path",
    "icon_eye_off_path",
    "icon_trash_path",
    "label_reserved_px"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_FilesInViewMenuItemWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
      10,   41, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   32,    2, 0x06,   11 /* Public */,
       4,    1,   35,    2, 0x06,   13 /* Public */,
       5,    1,   38,    2, 0x06,   15 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,

 // properties: name, type, flags
       6, QMetaType::QColor, 0x00015003, uint(-1), 0,
       7, QMetaType::QColor, 0x00015003, uint(-1), 0,
       8, QMetaType::QColor, 0x00015003, uint(-1), 0,
       9, QMetaType::QColor, 0x00015003, uint(-1), 0,
      10, QMetaType::QColor, 0x00015003, uint(-1), 0,
      11, QMetaType::Int, 0x00015003, uint(-1), 0,
      12, QMetaType::QString, 0x00015003, uint(-1), 0,
      13, QMetaType::QString, 0x00015003, uint(-1), 0,
      14, QMetaType::QString, 0x00015003, uint(-1), 0,
      15, QMetaType::Int, 0x00015003, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject FilesInViewMenuItemWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_FilesInViewMenuItemWidget.offsetsAndSizes,
    qt_meta_data_FilesInViewMenuItemWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_FilesInViewMenuItemWidget_t,
        // property 'icon_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'icon_color_hover'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'icon_color_eye'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'icon_color_eye_off'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'icon_color_trash'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'icon_px'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'icon_eye_path'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'icon_eye_off_path'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'icon_trash_path'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'label_reserved_px'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<FilesInViewMenuItemWidget, std::true_type>,
        // method 'show_only_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'toggle_visibility_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'remove_requested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void FilesInViewMenuItemWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FilesInViewMenuItemWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->show_only_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->toggle_visibility_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->remove_requested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FilesInViewMenuItemWidget::*)(const QString & );
            if (_t _q_method = &FilesInViewMenuItemWidget::show_only_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (FilesInViewMenuItemWidget::*)(const QString & );
            if (_t _q_method = &FilesInViewMenuItemWidget::toggle_visibility_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (FilesInViewMenuItemWidget::*)(const QString & );
            if (_t _q_method = &FilesInViewMenuItemWidget::remove_requested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<FilesInViewMenuItemWidget *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QColor*>(_v) = _t->get_icon_color(); break;
        case 1: *reinterpret_cast< QColor*>(_v) = _t->get_icon_color_hover(); break;
        case 2: *reinterpret_cast< QColor*>(_v) = _t->get_icon_color_eye(); break;
        case 3: *reinterpret_cast< QColor*>(_v) = _t->get_icon_color_eye_off(); break;
        case 4: *reinterpret_cast< QColor*>(_v) = _t->get_icon_color_trash(); break;
        case 5: *reinterpret_cast< int*>(_v) = _t->get_icon_px(); break;
        case 6: *reinterpret_cast< QString*>(_v) = _t->get_icon_eye_path(); break;
        case 7: *reinterpret_cast< QString*>(_v) = _t->get_icon_eye_off_path(); break;
        case 8: *reinterpret_cast< QString*>(_v) = _t->get_icon_trash_path(); break;
        case 9: *reinterpret_cast< int*>(_v) = _t->get_label_reserved_px(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<FilesInViewMenuItemWidget *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->set_icon_color(*reinterpret_cast< QColor*>(_v)); break;
        case 1: _t->set_icon_color_hover(*reinterpret_cast< QColor*>(_v)); break;
        case 2: _t->set_icon_color_eye(*reinterpret_cast< QColor*>(_v)); break;
        case 3: _t->set_icon_color_eye_off(*reinterpret_cast< QColor*>(_v)); break;
        case 4: _t->set_icon_color_trash(*reinterpret_cast< QColor*>(_v)); break;
        case 5: _t->set_icon_px(*reinterpret_cast< int*>(_v)); break;
        case 6: _t->set_icon_eye_path(*reinterpret_cast< QString*>(_v)); break;
        case 7: _t->set_icon_eye_off_path(*reinterpret_cast< QString*>(_v)); break;
        case 8: _t->set_icon_trash_path(*reinterpret_cast< QString*>(_v)); break;
        case 9: _t->set_label_reserved_px(*reinterpret_cast< int*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *FilesInViewMenuItemWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FilesInViewMenuItemWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FilesInViewMenuItemWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int FilesInViewMenuItemWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void FilesInViewMenuItemWidget::show_only_requested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void FilesInViewMenuItemWidget::toggle_visibility_requested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void FilesInViewMenuItemWidget::remove_requested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
