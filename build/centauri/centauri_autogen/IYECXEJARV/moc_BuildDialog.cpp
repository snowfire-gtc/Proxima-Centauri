/****************************************************************************
** Meta object code from reading C++ file 'BuildDialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/ui/dialogs/BuildDialog.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BuildDialog.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__BuildDialog_t {
    uint offsetsAndSizes[40];
    char stringdata0[21];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[8];
    char stringdata5[8];
    char stringdata6[15];
    char stringdata7[8];
    char stringdata8[11];
    char stringdata9[15];
    char stringdata10[13];
    char stringdata11[14];
    char stringdata12[14];
    char stringdata13[7];
    char stringdata14[13];
    char stringdata15[6];
    char stringdata16[16];
    char stringdata17[16];
    char stringdata18[10];
    char stringdata19[10];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__BuildDialog_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__BuildDialog_t qt_meta_stringdata_proxima__BuildDialog = {
    {
        QT_MOC_LITERAL(0, 20),  // "proxima::BuildDialog"
        QT_MOC_LITERAL(21, 12),  // "buildStarted"
        QT_MOC_LITERAL(34, 0),  // ""
        QT_MOC_LITERAL(35, 13),  // "buildProgress"
        QT_MOC_LITERAL(49, 7),  // "percent"
        QT_MOC_LITERAL(57, 7),  // "message"
        QT_MOC_LITERAL(65, 14),  // "buildCompleted"
        QT_MOC_LITERAL(80, 7),  // "success"
        QT_MOC_LITERAL(88, 10),  // "outputPath"
        QT_MOC_LITERAL(99, 14),  // "buildCancelled"
        QT_MOC_LITERAL(114, 12),  // "onStartBuild"
        QT_MOC_LITERAL(127, 13),  // "onCancelBuild"
        QT_MOC_LITERAL(141, 13),  // "onBuildOutput"
        QT_MOC_LITERAL(155, 6),  // "output"
        QT_MOC_LITERAL(162, 12),  // "onBuildError"
        QT_MOC_LITERAL(175, 5),  // "error"
        QT_MOC_LITERAL(181, 15),  // "onBuildProgress"
        QT_MOC_LITERAL(197, 15),  // "onBuildFinished"
        QT_MOC_LITERAL(213, 9),  // "onShowLog"
        QT_MOC_LITERAL(223, 9)   // "onSaveLog"
    },
    "proxima::BuildDialog",
    "buildStarted",
    "",
    "buildProgress",
    "percent",
    "message",
    "buildCompleted",
    "success",
    "outputPath",
    "buildCancelled",
    "onStartBuild",
    "onCancelBuild",
    "onBuildOutput",
    "output",
    "onBuildError",
    "error",
    "onBuildProgress",
    "onBuildFinished",
    "onShowLog",
    "onSaveLog"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__BuildDialog[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   86,    2, 0x06,    1 /* Public */,
       3,    2,   87,    2, 0x06,    2 /* Public */,
       6,    2,   92,    2, 0x06,    5 /* Public */,
       9,    0,   97,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      10,    0,   98,    2, 0x08,    9 /* Private */,
      11,    0,   99,    2, 0x08,   10 /* Private */,
      12,    1,  100,    2, 0x08,   11 /* Private */,
      14,    1,  103,    2, 0x08,   13 /* Private */,
      16,    2,  106,    2, 0x08,   15 /* Private */,
      17,    2,  111,    2, 0x08,   18 /* Private */,
      18,    0,  116,    2, 0x08,   21 /* Private */,
      19,    0,  117,    2, 0x08,   22 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    4,    5,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    7,    8,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void, QMetaType::QString,   15,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    4,    5,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    7,    8,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::BuildDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_proxima__BuildDialog.offsetsAndSizes,
    qt_meta_data_proxima__BuildDialog,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__BuildDialog_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<BuildDialog, std::true_type>,
        // method 'buildStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'buildProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'buildCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'buildCancelled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStartBuild'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCancelBuild'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBuildOutput'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onBuildError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onBuildProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onBuildFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onShowLog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSaveLog'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void proxima::BuildDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<BuildDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->buildStarted(); break;
        case 1: _t->buildProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 2: _t->buildCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 3: _t->buildCancelled(); break;
        case 4: _t->onStartBuild(); break;
        case 5: _t->onCancelBuild(); break;
        case 6: _t->onBuildOutput((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->onBuildError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->onBuildProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 9: _t->onBuildFinished((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 10: _t->onShowLog(); break;
        case 11: _t->onSaveLog(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (BuildDialog::*)();
            if (_t _q_method = &BuildDialog::buildStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (BuildDialog::*)(int , const QString & );
            if (_t _q_method = &BuildDialog::buildProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (BuildDialog::*)(bool , const QString & );
            if (_t _q_method = &BuildDialog::buildCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (BuildDialog::*)();
            if (_t _q_method = &BuildDialog::buildCancelled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *proxima::BuildDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::BuildDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__BuildDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int proxima::BuildDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void proxima::BuildDialog::buildStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void proxima::BuildDialog::buildProgress(int _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void proxima::BuildDialog::buildCompleted(bool _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void proxima::BuildDialog::buildCancelled()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
