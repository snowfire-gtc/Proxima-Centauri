/****************************************************************************
** Meta object code from reading C++ file 'SnippetManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/ui/editor/SnippetManager.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SnippetManager.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__SnippetManager_t {
    uint offsetsAndSizes[36];
    char stringdata0[24];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[15];
    char stringdata4[8];
    char stringdata5[13];
    char stringdata6[19];
    char stringdata7[14];
    char stringdata8[5];
    char stringdata9[21];
    char stringdata10[13];
    char stringdata11[6];
    char stringdata12[17];
    char stringdata13[26];
    char stringdata14[9];
    char stringdata15[13];
    char stringdata16[11];
    char stringdata17[24];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__SnippetManager_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__SnippetManager_t qt_meta_stringdata_proxima__SnippetManager = {
    {
        QT_MOC_LITERAL(0, 23),  // "proxima::SnippetManager"
        QT_MOC_LITERAL(24, 14),  // "sessionStarted"
        QT_MOC_LITERAL(39, 0),  // ""
        QT_MOC_LITERAL(40, 14),  // "SnippetSession"
        QT_MOC_LITERAL(55, 7),  // "session"
        QT_MOC_LITERAL(63, 12),  // "sessionEnded"
        QT_MOC_LITERAL(76, 18),  // "placeholderChanged"
        QT_MOC_LITERAL(95, 13),  // "placeholderId"
        QT_MOC_LITERAL(109, 4),  // "text"
        QT_MOC_LITERAL(114, 20),  // "placeholderNavigated"
        QT_MOC_LITERAL(135, 12),  // "currentIndex"
        QT_MOC_LITERAL(148, 5),  // "total"
        QT_MOC_LITERAL(154, 16),  // "snippetCompleted"
        QT_MOC_LITERAL(171, 25),  // "onDocumentContentsChanged"
        QT_MOC_LITERAL(197, 8),  // "position"
        QT_MOC_LITERAL(206, 12),  // "charsRemoved"
        QT_MOC_LITERAL(219, 10),  // "charsAdded"
        QT_MOC_LITERAL(230, 23)   // "onCursorPositionChanged"
    },
    "proxima::SnippetManager",
    "sessionStarted",
    "",
    "SnippetSession",
    "session",
    "sessionEnded",
    "placeholderChanged",
    "placeholderId",
    "text",
    "placeholderNavigated",
    "currentIndex",
    "total",
    "snippetCompleted",
    "onDocumentContentsChanged",
    "position",
    "charsRemoved",
    "charsAdded",
    "onCursorPositionChanged"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__SnippetManager[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   56,    2, 0x06,    1 /* Public */,
       5,    0,   59,    2, 0x06,    3 /* Public */,
       6,    2,   60,    2, 0x06,    4 /* Public */,
       9,    2,   65,    2, 0x06,    7 /* Public */,
      12,    0,   70,    2, 0x06,   10 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      13,    3,   71,    2, 0x08,   11 /* Private */,
      17,    0,   78,    2, 0x08,   15 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    7,    8,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   10,   11,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,   14,   15,   16,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::SnippetManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_proxima__SnippetManager.offsetsAndSizes,
    qt_meta_data_proxima__SnippetManager,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__SnippetManager_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SnippetManager, std::true_type>,
        // method 'sessionStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const SnippetSession &, std::false_type>,
        // method 'sessionEnded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'placeholderChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'placeholderNavigated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'snippetCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDocumentContentsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onCursorPositionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void proxima::SnippetManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SnippetManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sessionStarted((*reinterpret_cast< std::add_pointer_t<SnippetSession>>(_a[1]))); break;
        case 1: _t->sessionEnded(); break;
        case 2: _t->placeholderChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 3: _t->placeholderNavigated((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->snippetCompleted(); break;
        case 5: _t->onDocumentContentsChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 6: _t->onCursorPositionChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SnippetManager::*)(const SnippetSession & );
            if (_t _q_method = &SnippetManager::sessionStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SnippetManager::*)();
            if (_t _q_method = &SnippetManager::sessionEnded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SnippetManager::*)(int , const QString & );
            if (_t _q_method = &SnippetManager::placeholderChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SnippetManager::*)(int , int );
            if (_t _q_method = &SnippetManager::placeholderNavigated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (SnippetManager::*)();
            if (_t _q_method = &SnippetManager::snippetCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject *proxima::SnippetManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::SnippetManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__SnippetManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int proxima::SnippetManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void proxima::SnippetManager::sessionStarted(const SnippetSession & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void proxima::SnippetManager::sessionEnded()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void proxima::SnippetManager::placeholderChanged(int _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void proxima::SnippetManager::placeholderNavigated(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void proxima::SnippetManager::snippetCompleted()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
