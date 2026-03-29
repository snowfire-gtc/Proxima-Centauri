/****************************************************************************
** Meta object code from reading C++ file 'BreakpointManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/ui/debugger/BreakpointManager.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BreakpointManager.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__BreakpointManager_t {
    uint offsetsAndSizes[50];
    char stringdata0[27];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[3];
    char stringdata4[5];
    char stringdata5[5];
    char stringdata6[18];
    char stringdata7[18];
    char stringdata8[8];
    char stringdata9[27];
    char stringdata10[10];
    char stringdata11[14];
    char stringdata12[18];
    char stringdata13[5];
    char stringdata14[20];
    char stringdata15[14];
    char stringdata16[4];
    char stringdata17[16];
    char stringdata18[19];
    char stringdata19[19];
    char stringdata20[20];
    char stringdata21[11];
    char stringdata22[16];
    char stringdata23[7];
    char stringdata24[7];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__BreakpointManager_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__BreakpointManager_t qt_meta_stringdata_proxima__BreakpointManager = {
    {
        QT_MOC_LITERAL(0, 26),  // "proxima::BreakpointManager"
        QT_MOC_LITERAL(27, 15),  // "breakpointAdded"
        QT_MOC_LITERAL(43, 0),  // ""
        QT_MOC_LITERAL(44, 2),  // "id"
        QT_MOC_LITERAL(47, 4),  // "file"
        QT_MOC_LITERAL(52, 4),  // "line"
        QT_MOC_LITERAL(57, 17),  // "breakpointRemoved"
        QT_MOC_LITERAL(75, 17),  // "breakpointToggled"
        QT_MOC_LITERAL(93, 7),  // "enabled"
        QT_MOC_LITERAL(101, 26),  // "breakpointConditionChanged"
        QT_MOC_LITERAL(128, 9),  // "condition"
        QT_MOC_LITERAL(138, 13),  // "onItemChanged"
        QT_MOC_LITERAL(152, 17),  // "QTableWidgetItem*"
        QT_MOC_LITERAL(170, 4),  // "item"
        QT_MOC_LITERAL(175, 19),  // "onItemDoubleClicked"
        QT_MOC_LITERAL(195, 13),  // "onContextMenu"
        QT_MOC_LITERAL(209, 3),  // "pos"
        QT_MOC_LITERAL(213, 15),  // "onAddBreakpoint"
        QT_MOC_LITERAL(229, 18),  // "onRemoveBreakpoint"
        QT_MOC_LITERAL(248, 18),  // "onEnableBreakpoint"
        QT_MOC_LITERAL(267, 19),  // "onDisableBreakpoint"
        QT_MOC_LITERAL(287, 10),  // "onClearAll"
        QT_MOC_LITERAL(298, 15),  // "onEditCondition"
        QT_MOC_LITERAL(314, 6),  // "onSave"
        QT_MOC_LITERAL(321, 6)   // "onLoad"
    },
    "proxima::BreakpointManager",
    "breakpointAdded",
    "",
    "id",
    "file",
    "line",
    "breakpointRemoved",
    "breakpointToggled",
    "enabled",
    "breakpointConditionChanged",
    "condition",
    "onItemChanged",
    "QTableWidgetItem*",
    "item",
    "onItemDoubleClicked",
    "onContextMenu",
    "pos",
    "onAddBreakpoint",
    "onRemoveBreakpoint",
    "onEnableBreakpoint",
    "onDisableBreakpoint",
    "onClearAll",
    "onEditCondition",
    "onSave",
    "onLoad"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__BreakpointManager[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      15,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    3,  104,    2, 0x06,    1 /* Public */,
       6,    1,  111,    2, 0x06,    5 /* Public */,
       7,    2,  114,    2, 0x06,    7 /* Public */,
       9,    2,  119,    2, 0x06,   10 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      11,    1,  124,    2, 0x08,   13 /* Private */,
      14,    1,  127,    2, 0x08,   15 /* Private */,
      15,    1,  130,    2, 0x08,   17 /* Private */,
      17,    0,  133,    2, 0x08,   19 /* Private */,
      18,    0,  134,    2, 0x08,   20 /* Private */,
      19,    0,  135,    2, 0x08,   21 /* Private */,
      20,    0,  136,    2, 0x08,   22 /* Private */,
      21,    0,  137,    2, 0x08,   23 /* Private */,
      22,    0,  138,    2, 0x08,   24 /* Private */,
      23,    0,  139,    2, 0x08,   25 /* Private */,
      24,    0,  140,    2, 0x08,   26 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::QString, QMetaType::Int,    3,    4,    5,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    3,    8,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    3,   10,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 12,   13,
    QMetaType::Void, 0x80000000 | 12,   13,
    QMetaType::Void, QMetaType::QPoint,   16,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::BreakpointManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_proxima__BreakpointManager.offsetsAndSizes,
    qt_meta_data_proxima__BreakpointManager,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__BreakpointManager_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<BreakpointManager, std::true_type>,
        // method 'breakpointAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'breakpointRemoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'breakpointToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'breakpointConditionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onItemChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QTableWidgetItem *, std::false_type>,
        // method 'onItemDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QTableWidgetItem *, std::false_type>,
        // method 'onContextMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPoint &, std::false_type>,
        // method 'onAddBreakpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRemoveBreakpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEnableBreakpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDisableBreakpoint'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onClearAll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onEditCondition'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSave'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLoad'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void proxima::BreakpointManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<BreakpointManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->breakpointAdded((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 1: _t->breakpointRemoved((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->breakpointToggled((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 3: _t->breakpointConditionChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: _t->onItemChanged((*reinterpret_cast< std::add_pointer_t<QTableWidgetItem*>>(_a[1]))); break;
        case 5: _t->onItemDoubleClicked((*reinterpret_cast< std::add_pointer_t<QTableWidgetItem*>>(_a[1]))); break;
        case 6: _t->onContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 7: _t->onAddBreakpoint(); break;
        case 8: _t->onRemoveBreakpoint(); break;
        case 9: _t->onEnableBreakpoint(); break;
        case 10: _t->onDisableBreakpoint(); break;
        case 11: _t->onClearAll(); break;
        case 12: _t->onEditCondition(); break;
        case 13: _t->onSave(); break;
        case 14: _t->onLoad(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (BreakpointManager::*)(int , const QString & , int );
            if (_t _q_method = &BreakpointManager::breakpointAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (BreakpointManager::*)(int );
            if (_t _q_method = &BreakpointManager::breakpointRemoved; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (BreakpointManager::*)(int , bool );
            if (_t _q_method = &BreakpointManager::breakpointToggled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (BreakpointManager::*)(int , const QString & );
            if (_t _q_method = &BreakpointManager::breakpointConditionChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *proxima::BreakpointManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::BreakpointManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__BreakpointManager.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int proxima::BreakpointManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void proxima::BreakpointManager::breakpointAdded(int _t1, const QString & _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void proxima::BreakpointManager::breakpointRemoved(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void proxima::BreakpointManager::breakpointToggled(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void proxima::BreakpointManager::breakpointConditionChanged(int _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
