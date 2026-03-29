/****************************************************************************
** Meta object code from reading C++ file 'GitService.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/services/git/GitService.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GitService.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__GitService_t {
    uint offsetsAndSizes[50];
    char stringdata0[20];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[7];
    char stringdata5[16];
    char stringdata6[8];
    char stringdata7[5];
    char stringdata8[14];
    char stringdata9[14];
    char stringdata10[14];
    char stringdata11[7];
    char stringdata12[14];
    char stringdata13[6];
    char stringdata14[17];
    char stringdata15[10];
    char stringdata16[18];
    char stringdata17[18];
    char stringdata18[9];
    char stringdata19[21];
    char stringdata20[11];
    char stringdata21[15];
    char stringdata22[23];
    char stringdata23[33];
    char stringdata24[32];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__GitService_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__GitService_t qt_meta_stringdata_proxima__GitService = {
    {
        QT_MOC_LITERAL(0, 19),  // "proxima::GitService"
        QT_MOC_LITERAL(20, 13),  // "statusUpdated"
        QT_MOC_LITERAL(34, 0),  // ""
        QT_MOC_LITERAL(35, 9),  // "GitStatus"
        QT_MOC_LITERAL(45, 6),  // "status"
        QT_MOC_LITERAL(52, 15),  // "commitCompleted"
        QT_MOC_LITERAL(68, 7),  // "success"
        QT_MOC_LITERAL(76, 4),  // "hash"
        QT_MOC_LITERAL(81, 13),  // "pushCompleted"
        QT_MOC_LITERAL(95, 13),  // "pullCompleted"
        QT_MOC_LITERAL(109, 13),  // "branchChanged"
        QT_MOC_LITERAL(123, 6),  // "branch"
        QT_MOC_LITERAL(130, 13),  // "errorOccurred"
        QT_MOC_LITERAL(144, 5),  // "error"
        QT_MOC_LITERAL(150, 16),  // "operationStarted"
        QT_MOC_LITERAL(167, 9),  // "operation"
        QT_MOC_LITERAL(177, 17),  // "operationFinished"
        QT_MOC_LITERAL(195, 17),  // "onProcessFinished"
        QT_MOC_LITERAL(213, 8),  // "exitCode"
        QT_MOC_LITERAL(222, 20),  // "QProcess::ExitStatus"
        QT_MOC_LITERAL(243, 10),  // "exitStatus"
        QT_MOC_LITERAL(254, 14),  // "onProcessError"
        QT_MOC_LITERAL(269, 22),  // "QProcess::ProcessError"
        QT_MOC_LITERAL(292, 32),  // "onProcessReadyReadStandardOutput"
        QT_MOC_LITERAL(325, 31)   // "onProcessReadyReadStandardError"
    },
    "proxima::GitService",
    "statusUpdated",
    "",
    "GitStatus",
    "status",
    "commitCompleted",
    "success",
    "hash",
    "pushCompleted",
    "pullCompleted",
    "branchChanged",
    "branch",
    "errorOccurred",
    "error",
    "operationStarted",
    "operation",
    "operationFinished",
    "onProcessFinished",
    "exitCode",
    "QProcess::ExitStatus",
    "exitStatus",
    "onProcessError",
    "QProcess::ProcessError",
    "onProcessReadyReadStandardOutput",
    "onProcessReadyReadStandardError"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__GitService[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   86,    2, 0x06,    1 /* Public */,
       5,    2,   89,    2, 0x06,    3 /* Public */,
       8,    1,   94,    2, 0x06,    6 /* Public */,
       9,    1,   97,    2, 0x06,    8 /* Public */,
      10,    1,  100,    2, 0x06,   10 /* Public */,
      12,    1,  103,    2, 0x06,   12 /* Public */,
      14,    1,  106,    2, 0x06,   14 /* Public */,
      16,    1,  109,    2, 0x06,   16 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      17,    2,  112,    2, 0x08,   18 /* Private */,
      21,    1,  117,    2, 0x08,   21 /* Private */,
      23,    0,  120,    2, 0x08,   23 /* Private */,
      24,    0,  121,    2, 0x08,   24 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    6,    7,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void, QMetaType::QString,   15,
    QMetaType::Void, QMetaType::QString,   15,

 // slots: parameters
    QMetaType::Void, QMetaType::Int, 0x80000000 | 19,   18,   20,
    QMetaType::Void, 0x80000000 | 22,   13,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::GitService::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_proxima__GitService.offsetsAndSizes,
    qt_meta_data_proxima__GitService,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__GitService_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<GitService, std::true_type>,
        // method 'statusUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const GitStatus &, std::false_type>,
        // method 'commitCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'pushCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'pullCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'branchChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'operationStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'operationFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onProcessFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<QProcess::ExitStatus, std::false_type>,
        // method 'onProcessError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QProcess::ProcessError, std::false_type>,
        // method 'onProcessReadyReadStandardOutput'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onProcessReadyReadStandardError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void proxima::GitService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<GitService *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->statusUpdated((*reinterpret_cast< std::add_pointer_t<GitStatus>>(_a[1]))); break;
        case 1: _t->commitCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 2: _t->pushCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->pullCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->branchChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->operationStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->operationFinished((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->onProcessFinished((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QProcess::ExitStatus>>(_a[2]))); break;
        case 9: _t->onProcessError((*reinterpret_cast< std::add_pointer_t<QProcess::ProcessError>>(_a[1]))); break;
        case 10: _t->onProcessReadyReadStandardOutput(); break;
        case 11: _t->onProcessReadyReadStandardError(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (GitService::*)(const GitStatus & );
            if (_t _q_method = &GitService::statusUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (GitService::*)(bool , const QString & );
            if (_t _q_method = &GitService::commitCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (GitService::*)(bool );
            if (_t _q_method = &GitService::pushCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (GitService::*)(bool );
            if (_t _q_method = &GitService::pullCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (GitService::*)(const QString & );
            if (_t _q_method = &GitService::branchChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (GitService::*)(const QString & );
            if (_t _q_method = &GitService::errorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (GitService::*)(const QString & );
            if (_t _q_method = &GitService::operationStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (GitService::*)(const QString & );
            if (_t _q_method = &GitService::operationFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
    }
}

const QMetaObject *proxima::GitService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::GitService::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__GitService.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int proxima::GitService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void proxima::GitService::statusUpdated(const GitStatus & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void proxima::GitService::commitCompleted(bool _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void proxima::GitService::pushCompleted(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void proxima::GitService::pullCompleted(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void proxima::GitService::branchChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void proxima::GitService::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void proxima::GitService::operationStarted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void proxima::GitService::operationFinished(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
