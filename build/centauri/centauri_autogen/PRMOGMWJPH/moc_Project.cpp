/****************************************************************************
** Meta object code from reading C++ file 'Project.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/core/Project.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Project.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__Project_t {
    uint offsetsAndSizes[42];
    char stringdata0[17];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[14];
    char stringdata5[13];
    char stringdata6[14];
    char stringdata7[12];
    char stringdata8[8];
    char stringdata9[7];
    char stringdata10[14];
    char stringdata11[15];
    char stringdata12[13];
    char stringdata13[15];
    char stringdata14[8];
    char stringdata15[14];
    char stringdata16[14];
    char stringdata17[7];
    char stringdata18[12];
    char stringdata19[12];
    char stringdata20[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__Project_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__Project_t qt_meta_stringdata_proxima__Project = {
    {
        QT_MOC_LITERAL(0, 16),  // "proxima::Project"
        QT_MOC_LITERAL(17, 14),  // "projectCreated"
        QT_MOC_LITERAL(32, 0),  // ""
        QT_MOC_LITERAL(33, 4),  // "path"
        QT_MOC_LITERAL(38, 13),  // "projectLoaded"
        QT_MOC_LITERAL(52, 12),  // "projectSaved"
        QT_MOC_LITERAL(65, 13),  // "projectClosed"
        QT_MOC_LITERAL(79, 11),  // "moduleAdded"
        QT_MOC_LITERAL(91, 7),  // "Module*"
        QT_MOC_LITERAL(99, 6),  // "module"
        QT_MOC_LITERAL(106, 13),  // "moduleRemoved"
        QT_MOC_LITERAL(120, 14),  // "moduleModified"
        QT_MOC_LITERAL(135, 12),  // "buildStarted"
        QT_MOC_LITERAL(148, 14),  // "buildCompleted"
        QT_MOC_LITERAL(163, 7),  // "success"
        QT_MOC_LITERAL(171, 13),  // "statusChanged"
        QT_MOC_LITERAL(185, 13),  // "ProjectStatus"
        QT_MOC_LITERAL(199, 6),  // "status"
        QT_MOC_LITERAL(206, 11),  // "infoChanged"
        QT_MOC_LITERAL(218, 11),  // "ProjectInfo"
        QT_MOC_LITERAL(230, 4)   // "info"
    },
    "proxima::Project",
    "projectCreated",
    "",
    "path",
    "projectLoaded",
    "projectSaved",
    "projectClosed",
    "moduleAdded",
    "Module*",
    "module",
    "moduleRemoved",
    "moduleModified",
    "buildStarted",
    "buildCompleted",
    "success",
    "statusChanged",
    "ProjectStatus",
    "status",
    "infoChanged",
    "ProjectInfo",
    "info"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__Project[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      11,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   80,    2, 0x06,    1 /* Public */,
       4,    1,   83,    2, 0x06,    3 /* Public */,
       5,    0,   86,    2, 0x06,    5 /* Public */,
       6,    0,   87,    2, 0x06,    6 /* Public */,
       7,    1,   88,    2, 0x06,    7 /* Public */,
      10,    1,   91,    2, 0x06,    9 /* Public */,
      11,    1,   94,    2, 0x06,   11 /* Public */,
      12,    0,   97,    2, 0x06,   13 /* Public */,
      13,    1,   98,    2, 0x06,   14 /* Public */,
      15,    1,  101,    2, 0x06,   16 /* Public */,
      18,    1,  104,    2, 0x06,   18 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   14,
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void, 0x80000000 | 19,   20,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::Project::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_proxima__Project.offsetsAndSizes,
    qt_meta_data_proxima__Project,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__Project_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Project, std::true_type>,
        // method 'projectCreated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'projectLoaded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'projectSaved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'projectClosed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'moduleAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Module *, std::false_type>,
        // method 'moduleRemoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'moduleModified'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<Module *, std::false_type>,
        // method 'buildStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'buildCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'statusChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<ProjectStatus, std::false_type>,
        // method 'infoChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const ProjectInfo &, std::false_type>
    >,
    nullptr
} };

void proxima::Project::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Project *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->projectCreated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->projectLoaded((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->projectSaved(); break;
        case 3: _t->projectClosed(); break;
        case 4: _t->moduleAdded((*reinterpret_cast< std::add_pointer_t<Module*>>(_a[1]))); break;
        case 5: _t->moduleRemoved((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->moduleModified((*reinterpret_cast< std::add_pointer_t<Module*>>(_a[1]))); break;
        case 7: _t->buildStarted(); break;
        case 8: _t->buildCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 9: _t->statusChanged((*reinterpret_cast< std::add_pointer_t<ProjectStatus>>(_a[1]))); break;
        case 10: _t->infoChanged((*reinterpret_cast< std::add_pointer_t<ProjectInfo>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< Module* >(); break;
            }
            break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< Module* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Project::*)(const QString & );
            if (_t _q_method = &Project::projectCreated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Project::*)(const QString & );
            if (_t _q_method = &Project::projectLoaded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Project::*)();
            if (_t _q_method = &Project::projectSaved; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Project::*)();
            if (_t _q_method = &Project::projectClosed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Project::*)(Module * );
            if (_t _q_method = &Project::moduleAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (Project::*)(const QString & );
            if (_t _q_method = &Project::moduleRemoved; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (Project::*)(Module * );
            if (_t _q_method = &Project::moduleModified; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (Project::*)();
            if (_t _q_method = &Project::buildStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (Project::*)(bool );
            if (_t _q_method = &Project::buildCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (Project::*)(ProjectStatus );
            if (_t _q_method = &Project::statusChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (Project::*)(const ProjectInfo & );
            if (_t _q_method = &Project::infoChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
    }
}

const QMetaObject *proxima::Project::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::Project::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__Project.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int proxima::Project::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void proxima::Project::projectCreated(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void proxima::Project::projectLoaded(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void proxima::Project::projectSaved()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void proxima::Project::projectClosed()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void proxima::Project::moduleAdded(Module * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void proxima::Project::moduleRemoved(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void proxima::Project::moduleModified(Module * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void proxima::Project::buildStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void proxima::Project::buildCompleted(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void proxima::Project::statusChanged(ProjectStatus _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void proxima::Project::infoChanged(const ProjectInfo & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
