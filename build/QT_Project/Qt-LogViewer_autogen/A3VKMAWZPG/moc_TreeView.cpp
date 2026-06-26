/****************************************************************************
** Meta object code from reading C++ file 'TreeView.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Views/Shared/TreeView.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TreeView.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_TreeView_t {
    uint offsetsAndSizes[16];
    char stringdata0[9];
    char stringdata1[27];
    char stringdata2[29];
    char stringdata3[28];
    char stringdata4[25];
    char stringdata5[27];
    char stringdata6[28];
    char stringdata7[27];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_TreeView_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_TreeView_t qt_meta_stringdata_TreeView = {
    {
        QT_MOC_LITERAL(0, 8),  // "TreeView"
        QT_MOC_LITERAL(9, 26),  // "tree_view_branch_icon_open"
        QT_MOC_LITERAL(36, 28),  // "tree_view_branch_icon_closed"
        QT_MOC_LITERAL(65, 27),  // "tree_view_branch_icon_color"
        QT_MOC_LITERAL(93, 24),  // "tree_view_branch_icon_px"
        QT_MOC_LITERAL(118, 26),  // "tree_view_branch_pen_width"
        QT_MOC_LITERAL(145, 27),  // "tree_view_branch_hlength_px"
        QT_MOC_LITERAL(173, 26)   // "tree_view_branch_top_vline"
    },
    "TreeView",
    "tree_view_branch_icon_open",
    "tree_view_branch_icon_closed",
    "tree_view_branch_icon_color",
    "tree_view_branch_icon_px",
    "tree_view_branch_pen_width",
    "tree_view_branch_hlength_px",
    "tree_view_branch_top_vline"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_TreeView[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       7,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
       1, QMetaType::QString, 0x00015003, uint(-1), 0,
       2, QMetaType::QString, 0x00015003, uint(-1), 0,
       3, QMetaType::QColor, 0x00015003, uint(-1), 0,
       4, QMetaType::Int, 0x00015003, uint(-1), 0,
       5, QMetaType::Int, 0x00015003, uint(-1), 0,
       6, QMetaType::Int, 0x00015003, uint(-1), 0,
       7, QMetaType::Bool, 0x00015003, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject TreeView::staticMetaObject = { {
    QMetaObject::SuperData::link<QTreeView::staticMetaObject>(),
    qt_meta_stringdata_TreeView.offsetsAndSizes,
    qt_meta_data_TreeView,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_TreeView_t,
        // property 'tree_view_branch_icon_open'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'tree_view_branch_icon_closed'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'tree_view_branch_icon_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'tree_view_branch_icon_px'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'tree_view_branch_pen_width'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'tree_view_branch_hlength_px'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'tree_view_branch_top_vline'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TreeView, std::true_type>
    >,
    nullptr
} };

void TreeView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<TreeView *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->get_tree_view_branch_icon_open(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->get_tree_view_branch_icon_closed(); break;
        case 2: *reinterpret_cast< QColor*>(_v) = _t->get_tree_view_branch_icon_color(); break;
        case 3: *reinterpret_cast< int*>(_v) = _t->get_tree_view_branch_icon_px(); break;
        case 4: *reinterpret_cast< int*>(_v) = _t->get_tree_view_branch_pen_width(); break;
        case 5: *reinterpret_cast< int*>(_v) = _t->get_tree_view_branch_hlength_px(); break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->get_tree_view_branch_top_vline(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<TreeView *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->set_tree_view_branch_icon_open(*reinterpret_cast< QString*>(_v)); break;
        case 1: _t->set_tree_view_branch_icon_closed(*reinterpret_cast< QString*>(_v)); break;
        case 2: _t->set_tree_view_branch_icon_color(*reinterpret_cast< QColor*>(_v)); break;
        case 3: _t->set_tree_view_branch_icon_px(*reinterpret_cast< int*>(_v)); break;
        case 4: _t->set_tree_view_branch_pen_width(*reinterpret_cast< int*>(_v)); break;
        case 5: _t->set_tree_view_branch_hlength_px(*reinterpret_cast< int*>(_v)); break;
        case 6: _t->set_tree_view_branch_top_vline(*reinterpret_cast< bool*>(_v)); break;
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

const QMetaObject *TreeView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TreeView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TreeView.stringdata0))
        return static_cast<void*>(this);
    return QTreeView::qt_metacast(_clname);
}

int TreeView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeView::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
