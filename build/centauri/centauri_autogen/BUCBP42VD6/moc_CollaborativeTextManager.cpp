/****************************************************************************
** Meta object code from reading C++ file 'CollaborativeTextManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/ui/editor/CollaborativeTextManager.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CollaborativeTextManager.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_centauri__ui__CollaborativeTextManager_t {
    uint offsetsAndSizes[44];
    char stringdata0[39];
    char stringdata1[21];
    char stringdata2[1];
    char stringdata3[9];
    char stringdata4[24];
    char stringdata5[7];
    char stringdata6[20];
    char stringdata7[7];
    char stringdata8[5];
    char stringdata9[7];
    char stringdata10[23];
    char stringdata11[10];
    char stringdata12[9];
    char stringdata13[8];
    char stringdata14[7];
    char stringdata15[14];
    char stringdata16[24];
    char stringdata17[19];
    char stringdata18[23];
    char stringdata19[22];
    char stringdata20[5];
    char stringdata21[25];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_centauri__ui__CollaborativeTextManager_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_centauri__ui__CollaborativeTextManager_t qt_meta_stringdata_centauri__ui__CollaborativeTextManager = {
    {
        QT_MOC_LITERAL(0, 38),  // "centauri::ui::CollaborativeTe..."
        QT_MOC_LITERAL(39, 20),  // "remoteChangeReceived"
        QT_MOC_LITERAL(60, 0),  // ""
        QT_MOC_LITERAL(61, 8),  // "filePath"
        QT_MOC_LITERAL(70, 23),  // "proxima::DocumentChange"
        QT_MOC_LITERAL(94, 6),  // "change"
        QT_MOC_LITERAL(101, 19),  // "remoteCursorUpdated"
        QT_MOC_LITERAL(121, 6),  // "userId"
        QT_MOC_LITERAL(128, 4),  // "line"
        QT_MOC_LITERAL(133, 6),  // "column"
        QT_MOC_LITERAL(140, 22),  // "remoteSelectionUpdated"
        QT_MOC_LITERAL(163, 9),  // "startLine"
        QT_MOC_LITERAL(173, 8),  // "startCol"
        QT_MOC_LITERAL(182, 7),  // "endLine"
        QT_MOC_LITERAL(190, 6),  // "endCol"
        QT_MOC_LITERAL(197, 13),  // "onTextChanged"
        QT_MOC_LITERAL(211, 23),  // "onCursorPositionChanged"
        QT_MOC_LITERAL(235, 18),  // "onSelectionChanged"
        QT_MOC_LITERAL(254, 22),  // "onRemoteChangeReceived"
        QT_MOC_LITERAL(277, 21),  // "onRemoteCursorUpdated"
        QT_MOC_LITERAL(299, 4),  // "file"
        QT_MOC_LITERAL(304, 24)   // "onRemoteSelectionUpdated"
    },
    "centauri::ui::CollaborativeTextManager",
    "remoteChangeReceived",
    "",
    "filePath",
    "proxima::DocumentChange",
    "change",
    "remoteCursorUpdated",
    "userId",
    "line",
    "column",
    "remoteSelectionUpdated",
    "startLine",
    "startCol",
    "endLine",
    "endCol",
    "onTextChanged",
    "onCursorPositionChanged",
    "onSelectionChanged",
    "onRemoteChangeReceived",
    "onRemoteCursorUpdated",
    "file",
    "onRemoteSelectionUpdated"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_centauri__ui__CollaborativeTextManager[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   68,    2, 0x06,    1 /* Public */,
       6,    3,   73,    2, 0x06,    4 /* Public */,
      10,    5,   80,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      15,    0,   91,    2, 0x08,   14 /* Private */,
      16,    0,   92,    2, 0x08,   15 /* Private */,
      17,    0,   93,    2, 0x08,   16 /* Private */,
      18,    1,   94,    2, 0x08,   17 /* Private */,
      19,    4,   97,    2, 0x08,   19 /* Private */,
      21,    6,  106,    2, 0x08,   24 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, 0x80000000 | 4,    3,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::Int,    7,    8,    9,
    QMetaType::Void, QMetaType::QString, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,    7,   11,   12,   13,   14,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::Int, QMetaType::Int,    7,   20,    8,    9,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,    7,   20,   11,   12,   13,   14,

       0        // eod
};

Q_CONSTINIT const QMetaObject centauri::ui::CollaborativeTextManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_centauri__ui__CollaborativeTextManager.offsetsAndSizes,
    qt_meta_data_centauri__ui__CollaborativeTextManager,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_centauri__ui__CollaborativeTextManager_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CollaborativeTextManager, std::true_type>,
        // method 'remoteChangeReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const proxima::DocumentChange &, std::false_type>,
        // method 'remoteCursorUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'remoteSelectionUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCursorPositionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSelectionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRemoteChangeReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const proxima::DocumentChange &, std::false_type>,
        // method 'onRemoteCursorUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onRemoteSelectionUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void centauri::ui::CollaborativeTextManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CollaborativeTextManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->remoteChangeReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<proxima::DocumentChange>>(_a[2]))); break;
        case 1: _t->remoteCursorUpdated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 2: _t->remoteSelectionUpdated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[5]))); break;
        case 3: _t->onTextChanged(); break;
        case 4: _t->onCursorPositionChanged(); break;
        case 5: _t->onSelectionChanged(); break;
        case 6: _t->onRemoteChangeReceived((*reinterpret_cast< std::add_pointer_t<proxima::DocumentChange>>(_a[1]))); break;
        case 7: _t->onRemoteCursorUpdated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[4]))); break;
        case 8: _t->onRemoteSelectionUpdated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[6]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CollaborativeTextManager::*)(const QString & , const proxima::DocumentChange & );
            if (_t _q_method = &CollaborativeTextManager::remoteChangeReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CollaborativeTextManager::*)(const QString & , int , int );
            if (_t _q_method = &CollaborativeTextManager::remoteCursorUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CollaborativeTextManager::*)(const QString & , int , int , int , int );
            if (_t _q_method = &CollaborativeTextManager::remoteSelectionUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *centauri::ui::CollaborativeTextManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *centauri::ui::CollaborativeTextManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_centauri__ui__CollaborativeTextManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int centauri::ui::CollaborativeTextManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void centauri::ui::CollaborativeTextManager::remoteChangeReceived(const QString & _t1, const proxima::DocumentChange & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void centauri::ui::CollaborativeTextManager::remoteCursorUpdated(const QString & _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void centauri::ui::CollaborativeTextManager::remoteSelectionUpdated(const QString & _t1, int _t2, int _t3, int _t4, int _t5)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t5))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
