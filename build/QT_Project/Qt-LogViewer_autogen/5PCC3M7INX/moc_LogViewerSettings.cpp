/****************************************************************************
** Meta object code from reading C++ file 'LogViewerSettings.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../QT_Project/Headers/Qt-LogViewer/Services/LogViewerSettings.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LogViewerSettings.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_LogViewerSettings_t {
    uint offsetsAndSizes[16];
    char stringdata0[18];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[20];
    char stringdata5[14];
    char stringdata6[13];
    char stringdata7[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_LogViewerSettings_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_LogViewerSettings_t qt_meta_stringdata_LogViewerSettings = {
    {
        QT_MOC_LITERAL(0, 17),  // "LogViewerSettings"
        QT_MOC_LITERAL(18, 19),  // "languageCodeChanged"
        QT_MOC_LITERAL(38, 0),  // ""
        QT_MOC_LITERAL(39, 13),  // "language_code"
        QT_MOC_LITERAL(53, 19),  // "languageNameChanged"
        QT_MOC_LITERAL(73, 13),  // "language_name"
        QT_MOC_LITERAL(87, 12),  // "themeChanged"
        QT_MOC_LITERAL(100, 10)   // "theme_name"
    },
    "LogViewerSettings",
    "languageCodeChanged",
    "",
    "language_code",
    "languageNameChanged",
    "language_name",
    "themeChanged",
    "theme_name"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_LogViewerSettings[] = {

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
       6,    1,   38,    2, 0x06,    5 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QString,    7,

       0        // eod
};

Q_CONSTINIT const QMetaObject LogViewerSettings::staticMetaObject = { {
    QMetaObject::SuperData::link<Settings::staticMetaObject>(),
    qt_meta_stringdata_LogViewerSettings.offsetsAndSizes,
    qt_meta_data_LogViewerSettings,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_LogViewerSettings_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LogViewerSettings, std::true_type>,
        // method 'languageCodeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'languageNameChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'themeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void LogViewerSettings::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LogViewerSettings *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->languageCodeChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->languageNameChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->themeChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LogViewerSettings::*)(const QString & );
            if (_t _q_method = &LogViewerSettings::languageCodeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LogViewerSettings::*)(const QString & );
            if (_t _q_method = &LogViewerSettings::languageNameChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LogViewerSettings::*)(const QString & );
            if (_t _q_method = &LogViewerSettings::themeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *LogViewerSettings::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LogViewerSettings::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LogViewerSettings.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QtWidgetsCommonLib::IUiPreferences"))
        return static_cast< QtWidgetsCommonLib::IUiPreferences*>(this);
    if (!strcmp(_clname, "de.adrianhelbig.IUiPreferences"))
        return static_cast< QtWidgetsCommonLib::IUiPreferences*>(this);
    return Settings::qt_metacast(_clname);
}

int LogViewerSettings::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Settings::qt_metacall(_c, _id, _a);
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
void LogViewerSettings::languageCodeChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void LogViewerSettings::languageNameChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void LogViewerSettings::themeChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
