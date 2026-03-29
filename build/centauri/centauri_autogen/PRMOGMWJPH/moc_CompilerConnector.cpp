/****************************************************************************
** Meta object code from reading C++ file 'CompilerConnector.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/core/CompilerConnector.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CompilerConnector.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__CompilerConnector_t {
    uint offsetsAndSizes[96];
    char stringdata0[27];
    char stringdata1[10];
    char stringdata2[1];
    char stringdata3[13];
    char stringdata4[19];
    char stringdata5[20];
    char stringdata6[8];
    char stringdata7[8];
    char stringdata8[16];
    char stringdata9[14];
    char stringdata10[7];
    char stringdata11[17];
    char stringdata12[16];
    char stringdata13[17];
    char stringdata14[17];
    char stringdata15[11];
    char stringdata16[11];
    char stringdata17[6];
    char stringdata18[14];
    char stringdata19[3];
    char stringdata20[5];
    char stringdata21[5];
    char stringdata22[17];
    char stringdata23[17];
    char stringdata24[9];
    char stringdata25[17];
    char stringdata26[22];
    char stringdata27[10];
    char stringdata28[17];
    char stringdata29[18];
    char stringdata30[6];
    char stringdata31[15];
    char stringdata32[7];
    char stringdata33[14];
    char stringdata34[6];
    char stringdata35[16];
    char stringdata36[17];
    char stringdata37[18];
    char stringdata38[9];
    char stringdata39[21];
    char stringdata40[11];
    char stringdata41[26];
    char stringdata42[25];
    char stringdata43[8];
    char stringdata44[23];
    char stringdata45[14];
    char stringdata46[23];
    char stringdata47[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__CompilerConnector_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__CompilerConnector_t qt_meta_stringdata_proxima__CompilerConnector = {
    {
        QT_MOC_LITERAL(0, 26),  // "proxima::CompilerConnector"
        QT_MOC_LITERAL(27, 9),  // "connected"
        QT_MOC_LITERAL(37, 0),  // ""
        QT_MOC_LITERAL(38, 12),  // "disconnected"
        QT_MOC_LITERAL(51, 18),  // "compilationStarted"
        QT_MOC_LITERAL(70, 19),  // "compilationProgress"
        QT_MOC_LITERAL(90, 7),  // "percent"
        QT_MOC_LITERAL(98, 7),  // "message"
        QT_MOC_LITERAL(106, 15),  // "compileComplete"
        QT_MOC_LITERAL(122, 13),  // "CompileResult"
        QT_MOC_LITERAL(136, 6),  // "result"
        QT_MOC_LITERAL(143, 16),  // "executionStarted"
        QT_MOC_LITERAL(160, 15),  // "executionPaused"
        QT_MOC_LITERAL(176, 16),  // "executionResumed"
        QT_MOC_LITERAL(193, 16),  // "executionStopped"
        QT_MOC_LITERAL(210, 10),  // "debugEvent"
        QT_MOC_LITERAL(221, 10),  // "DebugEvent"
        QT_MOC_LITERAL(232, 5),  // "event"
        QT_MOC_LITERAL(238, 13),  // "breakpointHit"
        QT_MOC_LITERAL(252, 2),  // "id"
        QT_MOC_LITERAL(255, 4),  // "file"
        QT_MOC_LITERAL(260, 4),  // "line"
        QT_MOC_LITERAL(265, 16),  // "analysisComplete"
        QT_MOC_LITERAL(282, 16),  // "AnalysisResponse"
        QT_MOC_LITERAL(299, 8),  // "response"
        QT_MOC_LITERAL(308, 16),  // "variablesUpdated"
        QT_MOC_LITERAL(325, 21),  // "QMap<QString,QString>"
        QT_MOC_LITERAL(347, 9),  // "variables"
        QT_MOC_LITERAL(357, 16),  // "callStackUpdated"
        QT_MOC_LITERAL(374, 17),  // "QList<StackFrame>"
        QT_MOC_LITERAL(392, 5),  // "stack"
        QT_MOC_LITERAL(398, 14),  // "outputReceived"
        QT_MOC_LITERAL(413, 6),  // "output"
        QT_MOC_LITERAL(420, 13),  // "errorReceived"
        QT_MOC_LITERAL(434, 5),  // "error"
        QT_MOC_LITERAL(440, 15),  // "connectionError"
        QT_MOC_LITERAL(456, 16),  // "onProcessStarted"
        QT_MOC_LITERAL(473, 17),  // "onProcessFinished"
        QT_MOC_LITERAL(491, 8),  // "exitCode"
        QT_MOC_LITERAL(500, 20),  // "QProcess::ExitStatus"
        QT_MOC_LITERAL(521, 10),  // "exitStatus"
        QT_MOC_LITERAL(532, 25),  // "onReadyReadStandardOutput"
        QT_MOC_LITERAL(558, 24),  // "onReadyReadStandardError"
        QT_MOC_LITERAL(583, 7),  // "onError"
        QT_MOC_LITERAL(591, 22),  // "QProcess::ProcessError"
        QT_MOC_LITERAL(614, 13),  // "onStateChange"
        QT_MOC_LITERAL(628, 22),  // "QProcess::ProcessState"
        QT_MOC_LITERAL(651, 5)   // "state"
    },
    "proxima::CompilerConnector",
    "connected",
    "",
    "disconnected",
    "compilationStarted",
    "compilationProgress",
    "percent",
    "message",
    "compileComplete",
    "CompileResult",
    "result",
    "executionStarted",
    "executionPaused",
    "executionResumed",
    "executionStopped",
    "debugEvent",
    "DebugEvent",
    "event",
    "breakpointHit",
    "id",
    "file",
    "line",
    "analysisComplete",
    "AnalysisResponse",
    "response",
    "variablesUpdated",
    "QMap<QString,QString>",
    "variables",
    "callStackUpdated",
    "QList<StackFrame>",
    "stack",
    "outputReceived",
    "output",
    "errorReceived",
    "error",
    "connectionError",
    "onProcessStarted",
    "onProcessFinished",
    "exitCode",
    "QProcess::ExitStatus",
    "exitStatus",
    "onReadyReadStandardOutput",
    "onReadyReadStandardError",
    "onError",
    "QProcess::ProcessError",
    "onStateChange",
    "QProcess::ProcessState",
    "state"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__CompilerConnector[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      23,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      17,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  152,    2, 0x06,    1 /* Public */,
       3,    0,  153,    2, 0x06,    2 /* Public */,
       4,    0,  154,    2, 0x06,    3 /* Public */,
       5,    2,  155,    2, 0x06,    4 /* Public */,
       8,    1,  160,    2, 0x06,    7 /* Public */,
      11,    0,  163,    2, 0x06,    9 /* Public */,
      12,    0,  164,    2, 0x06,   10 /* Public */,
      13,    0,  165,    2, 0x06,   11 /* Public */,
      14,    0,  166,    2, 0x06,   12 /* Public */,
      15,    1,  167,    2, 0x06,   13 /* Public */,
      18,    3,  170,    2, 0x06,   15 /* Public */,
      22,    1,  177,    2, 0x06,   19 /* Public */,
      25,    1,  180,    2, 0x06,   21 /* Public */,
      28,    1,  183,    2, 0x06,   23 /* Public */,
      31,    1,  186,    2, 0x06,   25 /* Public */,
      33,    1,  189,    2, 0x06,   27 /* Public */,
      35,    1,  192,    2, 0x06,   29 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      36,    0,  195,    2, 0x08,   31 /* Private */,
      37,    2,  196,    2, 0x08,   32 /* Private */,
      41,    0,  201,    2, 0x08,   35 /* Private */,
      42,    0,  202,    2, 0x08,   36 /* Private */,
      43,    1,  203,    2, 0x08,   37 /* Private */,
      45,    1,  206,    2, 0x08,   39 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::QString,    6,    7,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void, QMetaType::Int, QMetaType::QString, QMetaType::Int,   19,   20,   21,
    QMetaType::Void, 0x80000000 | 23,   24,
    QMetaType::Void, 0x80000000 | 26,   27,
    QMetaType::Void, 0x80000000 | 29,   30,
    QMetaType::Void, QMetaType::QString,   32,
    QMetaType::Void, QMetaType::QString,   34,
    QMetaType::Void, QMetaType::QString,   34,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 39,   38,   40,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 44,   34,
    QMetaType::Void, 0x80000000 | 46,   47,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::CompilerConnector::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_proxima__CompilerConnector.offsetsAndSizes,
    qt_meta_data_proxima__CompilerConnector,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__CompilerConnector_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CompilerConnector, std::true_type>,
        // method 'connected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'disconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'compilationStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'compilationProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'compileComplete'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const CompileResult &, std::false_type>,
        // method 'executionStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'executionPaused'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'executionResumed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'executionStopped'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'debugEvent'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DebugEvent &, std::false_type>,
        // method 'breakpointHit'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'analysisComplete'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const AnalysisResponse &, std::false_type>,
        // method 'variablesUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QMap<QString,QString> &, std::false_type>,
        // method 'callStackUpdated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<StackFrame> &, std::false_type>,
        // method 'outputReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'errorReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'connectionError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onProcessStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onProcessFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<QProcess::ExitStatus, std::false_type>,
        // method 'onReadyReadStandardOutput'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onReadyReadStandardError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QProcess::ProcessError, std::false_type>,
        // method 'onStateChange'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QProcess::ProcessState, std::false_type>
    >,
    nullptr
} };

void proxima::CompilerConnector::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CompilerConnector *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->connected(); break;
        case 1: _t->disconnected(); break;
        case 2: _t->compilationStarted(); break;
        case 3: _t->compilationProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: _t->compileComplete((*reinterpret_cast< std::add_pointer_t<CompileResult>>(_a[1]))); break;
        case 5: _t->executionStarted(); break;
        case 6: _t->executionPaused(); break;
        case 7: _t->executionResumed(); break;
        case 8: _t->executionStopped(); break;
        case 9: _t->debugEvent((*reinterpret_cast< std::add_pointer_t<DebugEvent>>(_a[1]))); break;
        case 10: _t->breakpointHit((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        case 11: _t->analysisComplete((*reinterpret_cast< std::add_pointer_t<AnalysisResponse>>(_a[1]))); break;
        case 12: _t->variablesUpdated((*reinterpret_cast< std::add_pointer_t<QMap<QString,QString>>>(_a[1]))); break;
        case 13: _t->callStackUpdated((*reinterpret_cast< std::add_pointer_t<QList<StackFrame>>>(_a[1]))); break;
        case 14: _t->outputReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->errorReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->connectionError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: _t->onProcessStarted(); break;
        case 18: _t->onProcessFinished((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QProcess::ExitStatus>>(_a[2]))); break;
        case 19: _t->onReadyReadStandardOutput(); break;
        case 20: _t->onReadyReadStandardError(); break;
        case 21: _t->onError((*reinterpret_cast< std::add_pointer_t<QProcess::ProcessError>>(_a[1]))); break;
        case 22: _t->onStateChange((*reinterpret_cast< std::add_pointer_t<QProcess::ProcessState>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CompilerConnector::*)();
            if (_t _q_method = &CompilerConnector::connected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)();
            if (_t _q_method = &CompilerConnector::disconnected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)();
            if (_t _q_method = &CompilerConnector::compilationStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)(int , const QString & );
            if (_t _q_method = &CompilerConnector::compilationProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)(const CompileResult & );
            if (_t _q_method = &CompilerConnector::compileComplete; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)();
            if (_t _q_method = &CompilerConnector::executionStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)();
            if (_t _q_method = &CompilerConnector::executionPaused; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)();
            if (_t _q_method = &CompilerConnector::executionResumed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)();
            if (_t _q_method = &CompilerConnector::executionStopped; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)(const DebugEvent & );
            if (_t _q_method = &CompilerConnector::debugEvent; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)(int , const QString & , int );
            if (_t _q_method = &CompilerConnector::breakpointHit; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)(const AnalysisResponse & );
            if (_t _q_method = &CompilerConnector::analysisComplete; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)(const QMap<QString,QString> & );
            if (_t _q_method = &CompilerConnector::variablesUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)(const QVector<StackFrame> & );
            if (_t _q_method = &CompilerConnector::callStackUpdated; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)(const QString & );
            if (_t _q_method = &CompilerConnector::outputReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 14;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)(const QString & );
            if (_t _q_method = &CompilerConnector::errorReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 15;
                return;
            }
        }
        {
            using _t = void (CompilerConnector::*)(const QString & );
            if (_t _q_method = &CompilerConnector::connectionError; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 16;
                return;
            }
        }
    }
}

const QMetaObject *proxima::CompilerConnector::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::CompilerConnector::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__CompilerConnector.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int proxima::CompilerConnector::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 23)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 23;
    }
    return _id;
}

// SIGNAL 0
void proxima::CompilerConnector::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void proxima::CompilerConnector::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void proxima::CompilerConnector::compilationStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void proxima::CompilerConnector::compilationProgress(int _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void proxima::CompilerConnector::compileComplete(const CompileResult & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void proxima::CompilerConnector::executionStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void proxima::CompilerConnector::executionPaused()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void proxima::CompilerConnector::executionResumed()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void proxima::CompilerConnector::executionStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void proxima::CompilerConnector::debugEvent(const DebugEvent & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void proxima::CompilerConnector::breakpointHit(int _t1, const QString & _t2, int _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void proxima::CompilerConnector::analysisComplete(const AnalysisResponse & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void proxima::CompilerConnector::variablesUpdated(const QMap<QString,QString> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void proxima::CompilerConnector::callStackUpdated(const QVector<StackFrame> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void proxima::CompilerConnector::outputReceived(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void proxima::CompilerConnector::errorReceived(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void proxima::CompilerConnector::connectionError(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
