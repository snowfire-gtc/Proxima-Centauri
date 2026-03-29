/****************************************************************************
** Meta object code from reading C++ file 'VectorPlot.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/ui/debugger/visualizers/VectorPlot.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VectorPlot.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__VectorPlot_t {
    uint offsetsAndSizes[32];
    char stringdata0[20];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[6];
    char stringdata4[6];
    char stringdata5[19];
    char stringdata6[13];
    char stringdata7[17];
    char stringdata8[14];
    char stringdata9[6];
    char stringdata10[13];
    char stringdata11[12];
    char stringdata12[6];
    char stringdata13[12];
    char stringdata14[9];
    char stringdata15[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__VectorPlot_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__VectorPlot_t qt_meta_stringdata_proxima__VectorPlot = {
    {
        QT_MOC_LITERAL(0, 19),  // "proxima::VectorPlot"
        QT_MOC_LITERAL(20, 12),  // "pointClicked"
        QT_MOC_LITERAL(33, 0),  // ""
        QT_MOC_LITERAL(34, 5),  // "index"
        QT_MOC_LITERAL(40, 5),  // "value"
        QT_MOC_LITERAL(46, 18),  // "pointDoubleClicked"
        QT_MOC_LITERAL(65, 12),  // "pointHovered"
        QT_MOC_LITERAL(78, 16),  // "selectionChanged"
        QT_MOC_LITERAL(95, 13),  // "PlotSelection"
        QT_MOC_LITERAL(109, 5),  // "range"
        QT_MOC_LITERAL(115, 12),  // "dataModified"
        QT_MOC_LITERAL(128, 11),  // "zoomChanged"
        QT_MOC_LITERAL(140, 5),  // "level"
        QT_MOC_LITERAL(146, 11),  // "modeChanged"
        QT_MOC_LITERAL(158, 8),  // "PlotMode"
        QT_MOC_LITERAL(167, 4)   // "mode"
    },
    "proxima::VectorPlot",
    "pointClicked",
    "",
    "index",
    "value",
    "pointDoubleClicked",
    "pointHovered",
    "selectionChanged",
    "PlotSelection",
    "range",
    "dataModified",
    "zoomChanged",
    "level",
    "modeChanged",
    "PlotMode",
    "mode"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__VectorPlot[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   56,    2, 0x06,    1 /* Public */,
       5,    2,   61,    2, 0x06,    4 /* Public */,
       6,    2,   66,    2, 0x06,    7 /* Public */,
       7,    1,   71,    2, 0x06,   10 /* Public */,
      10,    0,   74,    2, 0x06,   12 /* Public */,
      11,    1,   75,    2, 0x06,   13 /* Public */,
      13,    1,   78,    2, 0x06,   15 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Double,    3,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Double,    3,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Double,    3,    4,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,   12,
    QMetaType::Void, 0x80000000 | 14,   15,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::VectorPlot::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_proxima__VectorPlot.offsetsAndSizes,
    qt_meta_data_proxima__VectorPlot,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__VectorPlot_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<VectorPlot, std::true_type>,
        // method 'pointClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'pointDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'pointHovered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'selectionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const PlotSelection &, std::false_type>,
        // method 'dataModified'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'zoomChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'modeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PlotMode, std::false_type>
    >,
    nullptr
} };

void proxima::VectorPlot::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VectorPlot *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->pointClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 1: _t->pointDoubleClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 2: _t->pointHovered((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 3: _t->selectionChanged((*reinterpret_cast< std::add_pointer_t<PlotSelection>>(_a[1]))); break;
        case 4: _t->dataModified(); break;
        case 5: _t->zoomChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 6: _t->modeChanged((*reinterpret_cast< std::add_pointer_t<PlotMode>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VectorPlot::*)(int , double );
            if (_t _q_method = &VectorPlot::pointClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (VectorPlot::*)(int , double );
            if (_t _q_method = &VectorPlot::pointDoubleClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (VectorPlot::*)(int , double );
            if (_t _q_method = &VectorPlot::pointHovered; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (VectorPlot::*)(const PlotSelection & );
            if (_t _q_method = &VectorPlot::selectionChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (VectorPlot::*)();
            if (_t _q_method = &VectorPlot::dataModified; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (VectorPlot::*)(double );
            if (_t _q_method = &VectorPlot::zoomChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (VectorPlot::*)(PlotMode );
            if (_t _q_method = &VectorPlot::modeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
    }
}

const QMetaObject *proxima::VectorPlot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::VectorPlot::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__VectorPlot.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int proxima::VectorPlot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
void proxima::VectorPlot::pointClicked(int _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void proxima::VectorPlot::pointDoubleClicked(int _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void proxima::VectorPlot::pointHovered(int _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void proxima::VectorPlot::selectionChanged(const PlotSelection & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void proxima::VectorPlot::dataModified()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void proxima::VectorPlot::zoomChanged(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void proxima::VectorPlot::modeChanged(PlotMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
