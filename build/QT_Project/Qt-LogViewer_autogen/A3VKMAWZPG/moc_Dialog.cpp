/****************************************************************************
** Meta object code from reading C++ file 'Dialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Views/Shared/Dialog.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Dialog.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_Dialog_t {
    uint offsetsAndSizes[14];
    char stringdata0[7];
    char stringdata1[14];
    char stringdata2[15];
    char stringdata3[17];
    char stringdata4[13];
    char stringdata5[13];
    char stringdata6[10];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_Dialog_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_Dialog_t qt_meta_stringdata_Dialog = {
    {
        QT_MOC_LITERAL(0, 6),  // "Dialog"
        QT_MOC_LITERAL(7, 13),  // "border_radius"
        QT_MOC_LITERAL(21, 14),  // "header_visible"
        QT_MOC_LITERAL(36, 16),  // "background_color"
        QT_MOC_LITERAL(53, 12),  // "border_color"
        QT_MOC_LITERAL(66, 12),  // "border_width"
        QT_MOC_LITERAL(79, 9)   // "resizable"
    },
    "Dialog",
    "border_radius",
    "header_visible",
    "background_color",
    "border_color",
    "border_width",
    "resizable"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_Dialog[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       6,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
       1, QMetaType::Int, 0x00015003, uint(-1), 0,
       2, QMetaType::Bool, 0x00015003, uint(-1), 0,
       3, QMetaType::QColor, 0x00015003, uint(-1), 0,
       4, QMetaType::QColor, 0x00015003, uint(-1), 0,
       5, QMetaType::Int, 0x00015003, uint(-1), 0,
       6, QMetaType::Bool, 0x00015003, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject Dialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_Dialog.offsetsAndSizes,
    qt_meta_data_Dialog,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_Dialog_t,
        // property 'border_radius'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'header_visible'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'background_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'border_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'border_width'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'resizable'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Dialog, std::true_type>
    >,
    nullptr
} };

void Dialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<Dialog *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = _t->get_border_radius(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->is_header_visible(); break;
        case 2: *reinterpret_cast< QColor*>(_v) = _t->get_background_color(); break;
        case 3: *reinterpret_cast< QColor*>(_v) = _t->get_border_color(); break;
        case 4: *reinterpret_cast< int*>(_v) = _t->get_border_width(); break;
        case 5: *reinterpret_cast< bool*>(_v) = _t->is_resizable(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<Dialog *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->set_border_radius(*reinterpret_cast< int*>(_v)); break;
        case 1: _t->set_header_visible(*reinterpret_cast< bool*>(_v)); break;
        case 2: _t->set_background_color(*reinterpret_cast< QColor*>(_v)); break;
        case 3: _t->set_border_color(*reinterpret_cast< QColor*>(_v)); break;
        case 4: _t->set_border_width(*reinterpret_cast< int*>(_v)); break;
        case 5: _t->set_resizable(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *Dialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Dialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Dialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int Dialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
