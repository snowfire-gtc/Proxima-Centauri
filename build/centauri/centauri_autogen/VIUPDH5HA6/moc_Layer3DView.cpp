/****************************************************************************
** Meta object code from reading C++ file 'Layer3DView.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/ui/debugger/visualizers/Layer3DView.h"
#include <QtGui/qtextcursor.h>
#include <QScreen>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Layer3DView.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__Layer3DView_t {
    uint offsetsAndSizes[40];
    char stringdata0[21];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[2];
    char stringdata4[2];
    char stringdata5[2];
    char stringdata6[6];
    char stringdata7[19];
    char stringdata8[13];
    char stringdata9[17];
    char stringdata10[13];
    char stringdata11[14];
    char stringdata12[13];
    char stringdata13[6];
    char stringdata14[9];
    char stringdata15[11];
    char stringdata16[4];
    char stringdata17[18];
    char stringdata18[11];
    char stringdata19[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__Layer3DView_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__Layer3DView_t qt_meta_stringdata_proxima__Layer3DView = {
    {
        QT_MOC_LITERAL(0, 20),  // "proxima::Layer3DView"
        QT_MOC_LITERAL(21, 12),  // "voxelClicked"
        QT_MOC_LITERAL(34, 0),  // ""
        QT_MOC_LITERAL(35, 1),  // "x"
        QT_MOC_LITERAL(37, 1),  // "y"
        QT_MOC_LITERAL(39, 1),  // "z"
        QT_MOC_LITERAL(41, 5),  // "value"
        QT_MOC_LITERAL(47, 18),  // "voxelDoubleClicked"
        QT_MOC_LITERAL(66, 12),  // "voxelHovered"
        QT_MOC_LITERAL(79, 16),  // "selectionChanged"
        QT_MOC_LITERAL(96, 12),  // "dataModified"
        QT_MOC_LITERAL(109, 13),  // "cameraChanged"
        QT_MOC_LITERAL(123, 12),  // "sliceChanged"
        QT_MOC_LITERAL(136, 5),  // "plane"
        QT_MOC_LITERAL(142, 8),  // "position"
        QT_MOC_LITERAL(151, 10),  // "fpsChanged"
        QT_MOC_LITERAL(162, 3),  // "fps"
        QT_MOC_LITERAL(166, 17),  // "renderModeChanged"
        QT_MOC_LITERAL(184, 10),  // "RenderMode"
        QT_MOC_LITERAL(195, 4)   // "mode"
    },
    "proxima::Layer3DView",
    "voxelClicked",
    "",
    "x",
    "y",
    "z",
    "value",
    "voxelDoubleClicked",
    "voxelHovered",
    "selectionChanged",
    "dataModified",
    "cameraChanged",
    "sliceChanged",
    "plane",
    "position",
    "fpsChanged",
    "fps",
    "renderModeChanged",
    "RenderMode",
    "mode"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__Layer3DView[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    4,   68,    2, 0x06,    1 /* Public */,
       7,    4,   77,    2, 0x06,    6 /* Public */,
       8,    4,   86,    2, 0x06,   11 /* Public */,
       9,    3,   95,    2, 0x06,   16 /* Public */,
      10,    0,  102,    2, 0x06,   20 /* Public */,
      11,    0,  103,    2, 0x06,   21 /* Public */,
      12,    2,  104,    2, 0x06,   22 /* Public */,
      15,    1,  109,    2, 0x06,   25 /* Public */,
      17,    1,  112,    2, 0x06,   27 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Double,    3,    4,    5,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Double,    3,    4,    5,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Double,    3,    4,    5,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,    3,    4,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   13,   14,
    QMetaType::Void, QMetaType::Int,   16,
    QMetaType::Void, 0x80000000 | 18,   19,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::Layer3DView::staticMetaObject = { {
    QMetaObject::SuperData::link<QOpenGLWidget::staticMetaObject>(),
    qt_meta_stringdata_proxima__Layer3DView.offsetsAndSizes,
    qt_meta_data_proxima__Layer3DView,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__Layer3DView_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Layer3DView, std::true_type>,
        // method 'voxelClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'voxelDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'voxelHovered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'selectionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'dataModified'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'cameraChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sliceChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'fpsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'renderModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<RenderMode, std::false_type>
    >,
    nullptr
} };

void proxima::Layer3DView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Layer3DView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->voxelClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4]))); break;
        case 1: _t->voxelDoubleClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4]))); break;
        case 2: _t->voxelHovered((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4]))); break;
        case 3: _t->selectionChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 4: _t->dataModified(); break;
        case 5: _t->cameraChanged(); break;
        case 6: _t->sliceChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 7: _t->fpsChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->renderModeChanged((*reinterpret_cast< std::add_pointer_t<RenderMode>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Layer3DView::*)(int , int , int , double );
            if (_t _q_method = &Layer3DView::voxelClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Layer3DView::*)(int , int , int , double );
            if (_t _q_method = &Layer3DView::voxelDoubleClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Layer3DView::*)(int , int , int , double );
            if (_t _q_method = &Layer3DView::voxelHovered; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Layer3DView::*)(int , int , int );
            if (_t _q_method = &Layer3DView::selectionChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Layer3DView::*)();
            if (_t _q_method = &Layer3DView::dataModified; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (Layer3DView::*)();
            if (_t _q_method = &Layer3DView::cameraChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (Layer3DView::*)(int , int );
            if (_t _q_method = &Layer3DView::sliceChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (Layer3DView::*)(int );
            if (_t _q_method = &Layer3DView::fpsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (Layer3DView::*)(RenderMode );
            if (_t _q_method = &Layer3DView::renderModeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
    }
}

const QMetaObject *proxima::Layer3DView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::Layer3DView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__Layer3DView.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QOpenGLFunctions"))
        return static_cast< QOpenGLFunctions*>(this);
    return QOpenGLWidget::qt_metacast(_clname);
}

int proxima::Layer3DView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QOpenGLWidget::qt_metacall(_c, _id, _a);
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
void proxima::Layer3DView::voxelClicked(int _t1, int _t2, int _t3, double _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void proxima::Layer3DView::voxelDoubleClicked(int _t1, int _t2, int _t3, double _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void proxima::Layer3DView::voxelHovered(int _t1, int _t2, int _t3, double _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void proxima::Layer3DView::selectionChanged(int _t1, int _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void proxima::Layer3DView::dataModified()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void proxima::Layer3DView::cameraChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void proxima::Layer3DView::sliceChanged(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void proxima::Layer3DView::fpsChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void proxima::Layer3DView::renderModeChanged(RenderMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
