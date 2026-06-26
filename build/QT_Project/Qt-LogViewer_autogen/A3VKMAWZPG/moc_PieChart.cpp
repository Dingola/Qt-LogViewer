/****************************************************************************
** Meta object code from reading C++ file 'PieChart.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Views/Shared/PieChart.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PieChart.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_PieChart_t {
    uint offsetsAndSizes[8];
    char stringdata0[9];
    char stringdata1[21];
    char stringdata2[18];
    char stringdata3[17];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_PieChart_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_PieChart_t qt_meta_stringdata_PieChart = {
    {
        QT_MOC_LITERAL(0, 8),  // "PieChart"
        QT_MOC_LITERAL(9, 20),  // "inner_radius_percent"
        QT_MOC_LITERAL(30, 17),  // "segment_gap_angle"
        QT_MOC_LITERAL(48, 16)   // "empty_ring_color"
    },
    "PieChart",
    "inner_radius_percent",
    "segment_gap_angle",
    "empty_ring_color"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_PieChart[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       3,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
       1, QMetaType::Int, 0x00015003, uint(-1), 0,
       2, QMetaType::Int, 0x00015003, uint(-1), 0,
       3, QMetaType::QColor, 0x00015003, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject PieChart::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_PieChart.offsetsAndSizes,
    qt_meta_data_PieChart,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_PieChart_t,
        // property 'inner_radius_percent'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'segment_gap_angle'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'empty_ring_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PieChart, std::true_type>
    >,
    nullptr
} };

void PieChart::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<PieChart *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = _t->get_inner_radius_percent(); break;
        case 1: *reinterpret_cast< int*>(_v) = _t->get_segment_gap_angle(); break;
        case 2: *reinterpret_cast< QColor*>(_v) = _t->get_empty_ring_color(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<PieChart *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->set_inner_radius_percent(*reinterpret_cast< int*>(_v)); break;
        case 1: _t->set_segment_gap_angle(*reinterpret_cast< int*>(_v)); break;
        case 2: _t->set_empty_ring_color(*reinterpret_cast< QColor*>(_v)); break;
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

const QMetaObject *PieChart::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PieChart::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PieChart.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int PieChart::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
