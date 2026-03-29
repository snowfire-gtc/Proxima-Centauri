/****************************************************************************
** Meta object code from reading C++ file 'ConsoleWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/ui/console/ConsoleWidget.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ConsoleWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__ConsoleTab_t {
    uint offsetsAndSizes[2];
    char stringdata0[20];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__ConsoleTab_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__ConsoleTab_t qt_meta_stringdata_proxima__ConsoleTab = {
    {
        QT_MOC_LITERAL(0, 19)   // "proxima::ConsoleTab"
    },
    "proxima::ConsoleTab"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__ConsoleTab[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::ConsoleTab::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_proxima__ConsoleTab.offsetsAndSizes,
    qt_meta_data_proxima__ConsoleTab,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__ConsoleTab_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ConsoleTab, std::true_type>
    >,
    nullptr
} };

void proxima::ConsoleTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *proxima::ConsoleTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::ConsoleTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__ConsoleTab.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int proxima::ConsoleTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_stringdata_proxima__REPLConsoleTab_t {
    uint offsetsAndSizes[22];
    char stringdata0[24];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[6];
    char stringdata4[17];
    char stringdata5[7];
    char stringdata6[16];
    char stringdata7[6];
    char stringdata8[18];
    char stringdata9[17];
    char stringdata10[8];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__REPLConsoleTab_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__REPLConsoleTab_t qt_meta_stringdata_proxima__REPLConsoleTab = {
    {
        QT_MOC_LITERAL(0, 23),  // "proxima::REPLConsoleTab"
        QT_MOC_LITERAL(24, 16),  // "onInputSubmitted"
        QT_MOC_LITERAL(41, 0),  // ""
        QT_MOC_LITERAL(42, 5),  // "input"
        QT_MOC_LITERAL(48, 16),  // "onOutputReceived"
        QT_MOC_LITERAL(65, 6),  // "output"
        QT_MOC_LITERAL(72, 15),  // "onErrorReceived"
        QT_MOC_LITERAL(88, 5),  // "error"
        QT_MOC_LITERAL(94, 17),  // "onPromptDisplayed"
        QT_MOC_LITERAL(112, 16),  // "onCommandHistory"
        QT_MOC_LITERAL(129, 7)   // "history"
    },
    "proxima::REPLConsoleTab",
    "onInputSubmitted",
    "",
    "input",
    "onOutputReceived",
    "output",
    "onErrorReceived",
    "error",
    "onPromptDisplayed",
    "onCommandHistory",
    "history"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__REPLConsoleTab[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   44,    2, 0x08,    1 /* Private */,
       4,    1,   47,    2, 0x08,    3 /* Private */,
       6,    1,   50,    2, 0x08,    5 /* Private */,
       8,    0,   53,    2, 0x08,    7 /* Private */,
       9,    1,   54,    2, 0x08,    8 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QStringList,   10,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::REPLConsoleTab::staticMetaObject = { {
    QMetaObject::SuperData::link<ConsoleTab::staticMetaObject>(),
    qt_meta_stringdata_proxima__REPLConsoleTab.offsetsAndSizes,
    qt_meta_data_proxima__REPLConsoleTab,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__REPLConsoleTab_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<REPLConsoleTab, std::true_type>,
        // method 'onInputSubmitted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onOutputReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onErrorReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onPromptDisplayed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCommandHistory'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QStringList &, std::false_type>
    >,
    nullptr
} };

void proxima::REPLConsoleTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<REPLConsoleTab *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onInputSubmitted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->onOutputReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->onErrorReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->onPromptDisplayed(); break;
        case 4: _t->onCommandHistory((*reinterpret_cast< std::add_pointer_t<QStringList>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *proxima::REPLConsoleTab::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::REPLConsoleTab::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__REPLConsoleTab.stringdata0))
        return static_cast<void*>(this);
    return ConsoleTab::qt_metacast(_clname);
}

int proxima::REPLConsoleTab::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ConsoleTab::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}
namespace {
struct qt_meta_stringdata_proxima__ConsoleWidget_t {
    uint offsetsAndSizes[48];
    char stringdata0[23];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[5];
    char stringdata5[23];
    char stringdata6[11];
    char stringdata7[20];
    char stringdata8[6];
    char stringdata9[20];
    char stringdata10[8];
    char stringdata11[7];
    char stringdata12[16];
    char stringdata13[13];
    char stringdata14[6];
    char stringdata15[14];
    char stringdata16[4];
    char stringdata17[11];
    char stringdata18[10];
    char stringdata19[10];
    char stringdata20[16];
    char stringdata21[7];
    char stringdata22[22];
    char stringdata23[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__ConsoleWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__ConsoleWidget_t qt_meta_stringdata_proxima__ConsoleWidget = {
    {
        QT_MOC_LITERAL(0, 22),  // "proxima::ConsoleWidget"
        QT_MOC_LITERAL(23, 12),  // "errorClicked"
        QT_MOC_LITERAL(36, 0),  // ""
        QT_MOC_LITERAL(37, 4),  // "file"
        QT_MOC_LITERAL(42, 4),  // "line"
        QT_MOC_LITERAL(47, 22),  // "callStackFrameSelected"
        QT_MOC_LITERAL(70, 10),  // "frameIndex"
        QT_MOC_LITERAL(81, 19),  // "verboseLevelChanged"
        QT_MOC_LITERAL(101, 5),  // "level"
        QT_MOC_LITERAL(107, 19),  // "replCommandExecuted"
        QT_MOC_LITERAL(127, 7),  // "command"
        QT_MOC_LITERAL(135, 6),  // "result"
        QT_MOC_LITERAL(142, 15),  // "replInitialized"
        QT_MOC_LITERAL(158, 12),  // "onTabChanged"
        QT_MOC_LITERAL(171, 5),  // "index"
        QT_MOC_LITERAL(177, 13),  // "onContextMenu"
        QT_MOC_LITERAL(191, 3),  // "pos"
        QT_MOC_LITERAL(195, 10),  // "onClearTab"
        QT_MOC_LITERAL(206, 9),  // "onSaveTab"
        QT_MOC_LITERAL(216, 9),  // "onCopyTab"
        QT_MOC_LITERAL(226, 15),  // "onFilterChanged"
        QT_MOC_LITERAL(242, 6),  // "filter"
        QT_MOC_LITERAL(249, 21),  // "onREPLCommandExecuted"
        QT_MOC_LITERAL(271, 17)   // "onREPLInitialized"
    },
    "proxima::ConsoleWidget",
    "errorClicked",
    "",
    "file",
    "line",
    "callStackFrameSelected",
    "frameIndex",
    "verboseLevelChanged",
    "level",
    "replCommandExecuted",
    "command",
    "result",
    "replInitialized",
    "onTabChanged",
    "index",
    "onContextMenu",
    "pos",
    "onClearTab",
    "onSaveTab",
    "onCopyTab",
    "onFilterChanged",
    "filter",
    "onREPLCommandExecuted",
    "onREPLInitialized"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__ConsoleWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   92,    2, 0x06,    1 /* Public */,
       5,    1,   97,    2, 0x06,    4 /* Public */,
       7,    1,  100,    2, 0x06,    6 /* Public */,
       9,    2,  103,    2, 0x06,    8 /* Public */,
      12,    0,  108,    2, 0x06,   11 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      13,    1,  109,    2, 0x08,   12 /* Private */,
      15,    1,  112,    2, 0x08,   14 /* Private */,
      17,    0,  115,    2, 0x08,   16 /* Private */,
      18,    0,  116,    2, 0x08,   17 /* Private */,
      19,    0,  117,    2, 0x08,   18 /* Private */,
      20,    1,  118,    2, 0x08,   19 /* Private */,
      22,    2,  121,    2, 0x08,   21 /* Private */,
      23,    0,  126,    2, 0x08,   24 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Int,    3,    4,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    8,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   10,   11,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,   14,
    QMetaType::Void, QMetaType::QPoint,   16,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   21,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   10,   11,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::ConsoleWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_proxima__ConsoleWidget.offsetsAndSizes,
    qt_meta_data_proxima__ConsoleWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__ConsoleWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ConsoleWidget, std::true_type>,
        // method 'errorClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'callStackFrameSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'verboseLevelChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'replCommandExecuted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'replInitialized'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTabChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onContextMenu'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPoint &, std::false_type>,
        // method 'onClearTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSaveTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCopyTab'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFilterChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onREPLCommandExecuted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onREPLInitialized'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void proxima::ConsoleWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ConsoleWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->errorClicked((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->callStackFrameSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->verboseLevelChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->replCommandExecuted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: _t->replInitialized(); break;
        case 5: _t->onTabChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->onContextMenu((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 7: _t->onClearTab(); break;
        case 8: _t->onSaveTab(); break;
        case 9: _t->onCopyTab(); break;
        case 10: _t->onFilterChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->onREPLCommandExecuted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 12: _t->onREPLInitialized(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ConsoleWidget::*)(const QString & , int );
            if (_t _q_method = &ConsoleWidget::errorClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ConsoleWidget::*)(int );
            if (_t _q_method = &ConsoleWidget::callStackFrameSelected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ConsoleWidget::*)(int );
            if (_t _q_method = &ConsoleWidget::verboseLevelChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ConsoleWidget::*)(const QString & , const QString & );
            if (_t _q_method = &ConsoleWidget::replCommandExecuted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ConsoleWidget::*)();
            if (_t _q_method = &ConsoleWidget::replInitialized; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject *proxima::ConsoleWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::ConsoleWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__ConsoleWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int proxima::ConsoleWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void proxima::ConsoleWidget::errorClicked(const QString & _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void proxima::ConsoleWidget::callStackFrameSelected(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void proxima::ConsoleWidget::verboseLevelChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void proxima::ConsoleWidget::replCommandExecuted(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void proxima::ConsoleWidget::replInitialized()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
