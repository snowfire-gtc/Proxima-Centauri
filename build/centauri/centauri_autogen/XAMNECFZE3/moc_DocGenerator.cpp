/****************************************************************************
** Meta object code from reading C++ file 'DocGenerator.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/docs/DocGenerator.h"
#include <QtGui/qtextcursor.h>
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DocGenerator.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__DocGenerator_t {
    uint offsetsAndSizes[42];
    char stringdata0[22];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[19];
    char stringdata4[8];
    char stringdata5[6];
    char stringdata6[8];
    char stringdata7[20];
    char stringdata8[8];
    char stringdata9[14];
    char stringdata10[6];
    char stringdata11[16];
    char stringdata12[8];
    char stringdata13[10];
    char stringdata14[8];
    char stringdata15[5];
    char stringdata16[12];
    char stringdata17[10];
    char stringdata18[7];
    char stringdata19[14];
    char stringdata20[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__DocGenerator_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__DocGenerator_t qt_meta_stringdata_proxima__DocGenerator = {
    {
        QT_MOC_LITERAL(0, 21),  // "proxima::DocGenerator"
        QT_MOC_LITERAL(22, 17),  // "generationStarted"
        QT_MOC_LITERAL(40, 0),  // ""
        QT_MOC_LITERAL(41, 18),  // "generationProgress"
        QT_MOC_LITERAL(60, 7),  // "current"
        QT_MOC_LITERAL(68, 5),  // "total"
        QT_MOC_LITERAL(74, 7),  // "message"
        QT_MOC_LITERAL(82, 19),  // "generationCompleted"
        QT_MOC_LITERAL(102, 7),  // "success"
        QT_MOC_LITERAL(110, 13),  // "errorOccurred"
        QT_MOC_LITERAL(124, 5),  // "error"
        QT_MOC_LITERAL(130, 15),  // "warningOccurred"
        QT_MOC_LITERAL(146, 7),  // "warning"
        QT_MOC_LITERAL(154, 9),  // "itemAdded"
        QT_MOC_LITERAL(164, 7),  // "DocItem"
        QT_MOC_LITERAL(172, 4),  // "item"
        QT_MOC_LITERAL(177, 11),  // "moduleAdded"
        QT_MOC_LITERAL(189, 9),  // "DocModule"
        QT_MOC_LITERAL(199, 6),  // "module"
        QT_MOC_LITERAL(206, 13),  // "fileProcessed"
        QT_MOC_LITERAL(220, 4)   // "file"
    },
    "proxima::DocGenerator",
    "generationStarted",
    "",
    "generationProgress",
    "current",
    "total",
    "message",
    "generationCompleted",
    "success",
    "errorOccurred",
    "error",
    "warningOccurred",
    "warning",
    "itemAdded",
    "DocItem",
    "item",
    "moduleAdded",
    "DocModule",
    "module",
    "fileProcessed",
    "file"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__DocGenerator[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   62,    2, 0x06,    1 /* Public */,
       3,    3,   63,    2, 0x06,    2 /* Public */,
       7,    1,   70,    2, 0x06,    6 /* Public */,
       9,    1,   73,    2, 0x06,    8 /* Public */,
      11,    1,   76,    2, 0x06,   10 /* Public */,
      13,    1,   79,    2, 0x06,   12 /* Public */,
      16,    1,   82,    2, 0x06,   14 /* Public */,
      19,    1,   85,    2, 0x06,   16 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::QString,    4,    5,    6,
    QMetaType::Void, QMetaType::Bool,    8,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, 0x80000000 | 17,   18,
    QMetaType::Void, QMetaType::QString,   20,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::DocGenerator::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_proxima__DocGenerator.offsetsAndSizes,
    qt_meta_data_proxima__DocGenerator,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__DocGenerator_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DocGenerator, std::true_type>,
        // method 'generationStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'generationProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'generationCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'warningOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'itemAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DocItem &, std::false_type>,
        // method 'moduleAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DocModule &, std::false_type>,
        // method 'fileProcessed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void proxima::DocGenerator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DocGenerator *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->generationStarted(); break;
        case 1: _t->generationProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 2: _t->generationCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->warningOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->itemAdded((*reinterpret_cast< std::add_pointer_t<DocItem>>(_a[1]))); break;
        case 6: _t->moduleAdded((*reinterpret_cast< std::add_pointer_t<DocModule>>(_a[1]))); break;
        case 7: _t->fileProcessed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DocGenerator::*)();
            if (_t _q_method = &DocGenerator::generationStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DocGenerator::*)(int , int , const QString & );
            if (_t _q_method = &DocGenerator::generationProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DocGenerator::*)(bool );
            if (_t _q_method = &DocGenerator::generationCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DocGenerator::*)(const QString & );
            if (_t _q_method = &DocGenerator::errorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DocGenerator::*)(const QString & );
            if (_t _q_method = &DocGenerator::warningOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DocGenerator::*)(const DocItem & );
            if (_t _q_method = &DocGenerator::itemAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (DocGenerator::*)(const DocModule & );
            if (_t _q_method = &DocGenerator::moduleAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (DocGenerator::*)(const QString & );
            if (_t _q_method = &DocGenerator::fileProcessed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
    }
}

const QMetaObject *proxima::DocGenerator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::DocGenerator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__DocGenerator.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int proxima::DocGenerator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void proxima::DocGenerator::generationStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void proxima::DocGenerator::generationProgress(int _t1, int _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void proxima::DocGenerator::generationCompleted(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void proxima::DocGenerator::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void proxima::DocGenerator::warningOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void proxima::DocGenerator::itemAdded(const DocItem & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void proxima::DocGenerator::moduleAdded(const DocModule & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void proxima::DocGenerator::fileProcessed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
