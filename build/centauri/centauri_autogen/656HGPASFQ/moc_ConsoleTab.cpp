/****************************************************************************
** Meta object code from reading C++ file 'ConsoleTab.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/ui/console/ConsoleTab.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ConsoleTab.h' doesn't include <QObject>."
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
    uint offsetsAndSizes[34];
    char stringdata0[20];
    char stringdata1[12];
    char stringdata2[1];
    char stringdata3[13];
    char stringdata4[5];
    char stringdata5[12];
    char stringdata6[5];
    char stringdata7[21];
    char stringdata8[4];
    char stringdata9[14];
    char stringdata10[7];
    char stringdata11[16];
    char stringdata12[14];
    char stringdata13[19];
    char stringdata14[20];
    char stringdata15[11];
    char stringdata16[14];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__ConsoleTab_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__ConsoleTab_t qt_meta_stringdata_proxima__ConsoleTab = {
    {
        QT_MOC_LITERAL(0, 19),  // "proxima::ConsoleTab"
        QT_MOC_LITERAL(20, 11),  // "textChanged"
        QT_MOC_LITERAL(32, 0),  // ""
        QT_MOC_LITERAL(33, 12),  // "textSelected"
        QT_MOC_LITERAL(46, 4),  // "text"
        QT_MOC_LITERAL(51, 11),  // "linkClicked"
        QT_MOC_LITERAL(63, 4),  // "link"
        QT_MOC_LITERAL(68, 20),  // "contextMenuRequested"
        QT_MOC_LITERAL(89, 3),  // "pos"
        QT_MOC_LITERAL(93, 13),  // "filterChanged"
        QT_MOC_LITERAL(107, 6),  // "filter"
        QT_MOC_LITERAL(114, 15),  // "maxLinesReached"
        QT_MOC_LITERAL(130, 13),  // "onTextChanged"
        QT_MOC_LITERAL(144, 18),  // "onSelectionChanged"
        QT_MOC_LITERAL(163, 19),  // "onContextMenuAction"
        QT_MOC_LITERAL(183, 10),  // "onFindText"
        QT_MOC_LITERAL(194, 13)   // "onClearFilter"
    },
    "proxima::ConsoleTab",
    "textChanged",
    "",
    "textSelected",
    "text",
    "linkClicked",
    "link",
    "contextMenuRequested",
    "pos",
    "filterChanged",
    "filter",
    "maxLinesReached",
    "onTextChanged",
    "onSelectionChanged",
    "onContextMenuAction",
    "onFindText",
    "onClearFilter"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__ConsoleTab[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   80,    2, 0x06,    1 /* Public */,
       3,    1,   81,    2, 0x06,    2 /* Public */,
       5,    1,   84,    2, 0x06,    4 /* Public */,
       7,    1,   87,    2, 0x06,    6 /* Public */,
       9,    1,   90,    2, 0x06,    8 /* Public */,
      11,    0,   93,    2, 0x06,   10 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      12,    0,   94,    2, 0x09,   11 /* Protected */,
      13,    0,   95,    2, 0x09,   12 /* Protected */,
      14,    0,   96,    2, 0x08,   13 /* Private */,
      15,    0,   97,    2, 0x08,   14 /* Private */,
      16,    0,   98,    2, 0x08,   15 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QPoint,    8,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

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
        QtPrivate::TypeAndForceComplete<ConsoleTab, std::true_type>,
        // method 'textChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'textSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'linkClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'contextMenuRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPoint &, std::false_type>,
        // method 'filterChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'maxLinesReached'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSelectionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onContextMenuAction'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFindText'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onClearFilter'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void proxima::ConsoleTab::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ConsoleTab *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->textChanged(); break;
        case 1: _t->textSelected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->linkClicked((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->contextMenuRequested((*reinterpret_cast< std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 4: _t->filterChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->maxLinesReached(); break;
        case 6: _t->onTextChanged(); break;
        case 7: _t->onSelectionChanged(); break;
        case 8: _t->onContextMenuAction(); break;
        case 9: _t->onFindText(); break;
        case 10: _t->onClearFilter(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ConsoleTab::*)();
            if (_t _q_method = &ConsoleTab::textChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ConsoleTab::*)(const QString & );
            if (_t _q_method = &ConsoleTab::textSelected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ConsoleTab::*)(const QString & );
            if (_t _q_method = &ConsoleTab::linkClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ConsoleTab::*)(const QPoint & );
            if (_t _q_method = &ConsoleTab::contextMenuRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ConsoleTab::*)(const QString & );
            if (_t _q_method = &ConsoleTab::filterChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (ConsoleTab::*)();
            if (_t _q_method = &ConsoleTab::maxLinesReached; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
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
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void proxima::ConsoleTab::textChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void proxima::ConsoleTab::textSelected(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void proxima::ConsoleTab::linkClicked(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void proxima::ConsoleTab::contextMenuRequested(const QPoint & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void proxima::ConsoleTab::filterChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void proxima::ConsoleTab::maxLinesReached()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
