/****************************************************************************
** Meta object code from reading C++ file 'LLMService.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/services/llm/LLMService.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LLMService.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__LLMService_t {
    uint offsetsAndSizes[42];
    char stringdata0[20];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[22];
    char stringdata4[12];
    char stringdata5[17];
    char stringdata6[12];
    char stringdata7[16];
    char stringdata8[11];
    char stringdata9[14];
    char stringdata10[6];
    char stringdata11[18];
    char stringdata12[19];
    char stringdata13[24];
    char stringdata14[10];
    char stringdata15[23];
    char stringdata16[15];
    char stringdata17[6];
    char stringdata18[15];
    char stringdata19[28];
    char stringdata20[10];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__LLMService_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__LLMService_t qt_meta_stringdata_proxima__LLMService = {
    {
        QT_MOC_LITERAL(0, 19),  // "proxima::LLMService"
        QT_MOC_LITERAL(20, 16),  // "suggestionsReady"
        QT_MOC_LITERAL(37, 0),  // ""
        QT_MOC_LITERAL(38, 21),  // "QList<CodeSuggestion>"
        QT_MOC_LITERAL(60, 11),  // "suggestions"
        QT_MOC_LITERAL(72, 16),  // "explanationReady"
        QT_MOC_LITERAL(89, 11),  // "explanation"
        QT_MOC_LITERAL(101, 15),  // "completionReady"
        QT_MOC_LITERAL(117, 10),  // "completion"
        QT_MOC_LITERAL(128, 13),  // "errorOccurred"
        QT_MOC_LITERAL(142, 5),  // "error"
        QT_MOC_LITERAL(148, 17),  // "processingStarted"
        QT_MOC_LITERAL(166, 18),  // "processingFinished"
        QT_MOC_LITERAL(185, 23),  // "connectionStatusChanged"
        QT_MOC_LITERAL(209, 9),  // "connected"
        QT_MOC_LITERAL(219, 22),  // "onNetworkReplyFinished"
        QT_MOC_LITERAL(242, 14),  // "QNetworkReply*"
        QT_MOC_LITERAL(257, 5),  // "reply"
        QT_MOC_LITERAL(263, 14),  // "onNetworkError"
        QT_MOC_LITERAL(278, 27),  // "QNetworkReply::NetworkError"
        QT_MOC_LITERAL(306, 9)   // "onTimeout"
    },
    "proxima::LLMService",
    "suggestionsReady",
    "",
    "QList<CodeSuggestion>",
    "suggestions",
    "explanationReady",
    "explanation",
    "completionReady",
    "completion",
    "errorOccurred",
    "error",
    "processingStarted",
    "processingFinished",
    "connectionStatusChanged",
    "connected",
    "onNetworkReplyFinished",
    "QNetworkReply*",
    "reply",
    "onNetworkError",
    "QNetworkReply::NetworkError",
    "onTimeout"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__LLMService[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   74,    2, 0x06,    1 /* Public */,
       5,    1,   77,    2, 0x06,    3 /* Public */,
       7,    1,   80,    2, 0x06,    5 /* Public */,
       9,    1,   83,    2, 0x06,    7 /* Public */,
      11,    0,   86,    2, 0x06,    9 /* Public */,
      12,    0,   87,    2, 0x06,   10 /* Public */,
      13,    1,   88,    2, 0x06,   11 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      15,    1,   91,    2, 0x08,   13 /* Private */,
      18,    1,   94,    2, 0x08,   15 /* Private */,
      20,    0,   97,    2, 0x08,   17 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   14,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void, 0x80000000 | 19,   10,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::LLMService::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_proxima__LLMService.offsetsAndSizes,
    qt_meta_data_proxima__LLMService,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__LLMService_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LLMService, std::true_type>,
        // method 'suggestionsReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<CodeSuggestion> &, std::false_type>,
        // method 'explanationReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'completionReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'processingStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'processingFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'connectionStatusChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onNetworkReplyFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QNetworkReply *, std::false_type>,
        // method 'onNetworkError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QNetworkReply::NetworkError, std::false_type>,
        // method 'onTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void proxima::LLMService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LLMService *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->suggestionsReady((*reinterpret_cast< std::add_pointer_t<QList<CodeSuggestion>>>(_a[1]))); break;
        case 1: _t->explanationReady((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->completionReady((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->processingStarted(); break;
        case 5: _t->processingFinished(); break;
        case 6: _t->connectionStatusChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 7: _t->onNetworkReplyFinished((*reinterpret_cast< std::add_pointer_t<QNetworkReply*>>(_a[1]))); break;
        case 8: _t->onNetworkError((*reinterpret_cast< std::add_pointer_t<QNetworkReply::NetworkError>>(_a[1]))); break;
        case 9: _t->onTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QNetworkReply* >(); break;
            }
            break;
        case 8:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QNetworkReply::NetworkError >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (LLMService::*)(const QVector<CodeSuggestion> & );
            if (_t _q_method = &LLMService::suggestionsReady; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (LLMService::*)(const QString & );
            if (_t _q_method = &LLMService::explanationReady; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (LLMService::*)(const QString & );
            if (_t _q_method = &LLMService::completionReady; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (LLMService::*)(const QString & );
            if (_t _q_method = &LLMService::errorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (LLMService::*)();
            if (_t _q_method = &LLMService::processingStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (LLMService::*)();
            if (_t _q_method = &LLMService::processingFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (LLMService::*)(bool );
            if (_t _q_method = &LLMService::connectionStatusChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
    }
}

const QMetaObject *proxima::LLMService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::LLMService::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__LLMService.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int proxima::LLMService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void proxima::LLMService::suggestionsReady(const QVector<CodeSuggestion> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void proxima::LLMService::explanationReady(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void proxima::LLMService::completionReady(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void proxima::LLMService::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void proxima::LLMService::processingStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void proxima::LLMService::processingFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void proxima::LLMService::connectionStatusChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
