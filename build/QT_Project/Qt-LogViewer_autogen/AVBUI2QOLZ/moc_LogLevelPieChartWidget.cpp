/****************************************************************************
** Meta object code from reading C++ file 'LogLevelPieChartWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Views/App/LogLevelPieChartWidget.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LogLevelPieChartWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LogLevelPieChartWidget_t {
    uint offsetsAndSizes[18];
    char stringdata0[23];
    char stringdata1[12];
    char stringdata2[12];
    char stringdata3[11];
    char stringdata4[14];
    char stringdata5[12];
    char stringdata6[12];
    char stringdata7[21];
    char stringdata8[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LogLevelPieChartWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LogLevelPieChartWidget_t qt_meta_stringdata_LogLevelPieChartWidget = {
    {
        QT_MOC_LITERAL(0, 22),  // "LogLevelPieChartWidget"
        QT_MOC_LITERAL(23, 11),  // "trace_color"
        QT_MOC_LITERAL(35, 11),  // "debug_color"
        QT_MOC_LITERAL(47, 10),  // "info_color"
        QT_MOC_LITERAL(58, 13),  // "warning_color"
        QT_MOC_LITERAL(72, 11),  // "error_color"
        QT_MOC_LITERAL(84, 11),  // "fatal_color"
        QT_MOC_LITERAL(96, 20),  // "inner_radius_percent"
        QT_MOC_LITERAL(117, 17)   // "segment_gap_angle"
    },
    "LogLevelPieChartWidget",
    "trace_color",
    "debug_color",
    "info_color",
    "warning_color",
    "error_color",
    "fatal_color",
    "inner_radius_percent",
    "segment_gap_angle"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LogLevelPieChartWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       8,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
       1, QMetaType::QColor, 0x00015003, uint(-1), 0,
       2, QMetaType::QColor, 0x00015003, uint(-1), 0,
       3, QMetaType::QColor, 0x00015003, uint(-1), 0,
       4, QMetaType::QColor, 0x00015003, uint(-1), 0,
       5, QMetaType::QColor, 0x00015003, uint(-1), 0,
       6, QMetaType::QColor, 0x00015003, uint(-1), 0,
       7, QMetaType::Int, 0x00015003, uint(-1), 0,
       8, QMetaType::Int, 0x00015003, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject LogLevelPieChartWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_LogLevelPieChartWidget.offsetsAndSizes,
    qt_meta_data_LogLevelPieChartWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LogLevelPieChartWidget_t,
        // property 'trace_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'debug_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'info_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'warning_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'error_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'fatal_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'inner_radius_percent'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'segment_gap_angle'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LogLevelPieChartWidget, std::true_type>
    >,
    nullptr
} };

void LogLevelPieChartWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<LogLevelPieChartWidget *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QColor*>(_v) = _t->get_trace_color(); break;
        case 1: *reinterpret_cast< QColor*>(_v) = _t->get_debug_color(); break;
        case 2: *reinterpret_cast< QColor*>(_v) = _t->get_info_color(); break;
        case 3: *reinterpret_cast< QColor*>(_v) = _t->get_warning_color(); break;
        case 4: *reinterpret_cast< QColor*>(_v) = _t->get_error_color(); break;
        case 5: *reinterpret_cast< QColor*>(_v) = _t->get_fatal_color(); break;
        case 6: *reinterpret_cast< int*>(_v) = _t->get_inner_radius_percent(); break;
        case 7: *reinterpret_cast< int*>(_v) = _t->get_segment_gap_angle(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<LogLevelPieChartWidget *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->set_trace_color(*reinterpret_cast< QColor*>(_v)); break;
        case 1: _t->set_debug_color(*reinterpret_cast< QColor*>(_v)); break;
        case 2: _t->set_info_color(*reinterpret_cast< QColor*>(_v)); break;
        case 3: _t->set_warning_color(*reinterpret_cast< QColor*>(_v)); break;
        case 4: _t->set_error_color(*reinterpret_cast< QColor*>(_v)); break;
        case 5: _t->set_fatal_color(*reinterpret_cast< QColor*>(_v)); break;
        case 6: _t->set_inner_radius_percent(*reinterpret_cast< int*>(_v)); break;
        case 7: _t->set_segment_gap_angle(*reinterpret_cast< int*>(_v)); break;
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

const QMetaObject *LogLevelPieChartWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LogLevelPieChartWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LogLevelPieChartWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int LogLevelPieChartWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
