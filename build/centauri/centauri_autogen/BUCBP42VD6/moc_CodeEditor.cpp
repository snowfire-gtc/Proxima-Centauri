/****************************************************************************
** Meta object code from reading C++ file 'CodeEditor.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/ui/editor/CodeEditor.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CodeEditor.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__LineNumberArea_t {
    uint offsetsAndSizes[2];
    char stringdata0[24];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__LineNumberArea_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__LineNumberArea_t qt_meta_stringdata_proxima__LineNumberArea = {
    {
        QT_MOC_LITERAL(0, 23)   // "proxima::LineNumberArea"
    },
    "proxima::LineNumberArea"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__LineNumberArea[] = {

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

Q_CONSTINIT const QMetaObject proxima::LineNumberArea::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_proxima__LineNumberArea.offsetsAndSizes,
    qt_meta_data_proxima__LineNumberArea,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__LineNumberArea_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LineNumberArea, std::true_type>
    >,
    nullptr
} };

void proxima::LineNumberArea::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *proxima::LineNumberArea::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::LineNumberArea::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__LineNumberArea.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int proxima::LineNumberArea::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_stringdata_proxima__CodeEditor_t {
    uint offsetsAndSizes[74];
    char stringdata0[20];
    char stringdata1[22];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[7];
    char stringdata5[13];
    char stringdata6[9];
    char stringdata7[18];
    char stringdata8[8];
    char stringdata9[12];
    char stringdata10[7];
    char stringdata11[21];
    char stringdata12[14];
    char stringdata13[16];
    char stringdata14[8];
    char stringdata15[20];
    char stringdata16[23];
    char stringdata17[8];
    char stringdata18[17];
    char stringdata19[5];
    char stringdata20[6];
    char stringdata21[20];
    char stringdata22[13];
    char stringdata23[5];
    char stringdata24[26];
    char stringdata25[14];
    char stringdata26[21];
    char stringdata27[21];
    char stringdata28[5];
    char stringdata29[3];
    char stringdata30[26];
    char stringdata31[5];
    char stringdata32[19];
    char stringdata33[14];
    char stringdata34[9];
    char stringdata35[13];
    char stringdata36[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__CodeEditor_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__CodeEditor_t qt_meta_stringdata_proxima__CodeEditor = {
    {
        QT_MOC_LITERAL(0, 19),  // "proxima::CodeEditor"
        QT_MOC_LITERAL(20, 21),  // "cursorPositionChanged"
        QT_MOC_LITERAL(42, 0),  // ""
        QT_MOC_LITERAL(43, 4),  // "line"
        QT_MOC_LITERAL(48, 6),  // "column"
        QT_MOC_LITERAL(55, 12),  // "fileModified"
        QT_MOC_LITERAL(68, 8),  // "modified"
        QT_MOC_LITERAL(77, 17),  // "breakpointToggled"
        QT_MOC_LITERAL(95, 7),  // "enabled"
        QT_MOC_LITERAL(103, 11),  // "foldToggled"
        QT_MOC_LITERAL(115, 6),  // "folded"
        QT_MOC_LITERAL(122, 20),  // "executionLineChanged"
        QT_MOC_LITERAL(143, 13),  // "codeFormatted"
        QT_MOC_LITERAL(157, 15),  // "snippetInserted"
        QT_MOC_LITERAL(173, 7),  // "snippet"
        QT_MOC_LITERAL(181, 19),  // "completionRequested"
        QT_MOC_LITERAL(201, 22),  // "visualizationRequested"
        QT_MOC_LITERAL(224, 7),  // "varName"
        QT_MOC_LITERAL(232, 16),  // "authorRegistered"
        QT_MOC_LITERAL(249, 4),  // "name"
        QT_MOC_LITERAL(254, 5),  // "color"
        QT_MOC_LITERAL(260, 19),  // "lineEditInfoChanged"
        QT_MOC_LITERAL(280, 12),  // "LineEditInfo"
        QT_MOC_LITERAL(293, 4),  // "info"
        QT_MOC_LITERAL(298, 25),  // "updateLineNumberAreaWidth"
        QT_MOC_LITERAL(324, 13),  // "newBlockCount"
        QT_MOC_LITERAL(338, 20),  // "highlightCurrentLine"
        QT_MOC_LITERAL(359, 20),  // "updateLineNumberArea"
        QT_MOC_LITERAL(380, 4),  // "rect"
        QT_MOC_LITERAL(385, 2),  // "dy"
        QT_MOC_LITERAL(388, 25),  // "onCompletionItemActivated"
        QT_MOC_LITERAL(414, 4),  // "item"
        QT_MOC_LITERAL(419, 18),  // "onFoldTimerTimeout"
        QT_MOC_LITERAL(438, 13),  // "onTextChanged"
        QT_MOC_LITERAL(452, 8),  // "position"
        QT_MOC_LITERAL(461, 12),  // "charsRemoved"
        QT_MOC_LITERAL(474, 10)   // "charsAdded"
    },
    "proxima::CodeEditor",
    "cursorPositionChanged",
    "",
    "line",
    "column",
    "fileModified",
    "modified",
    "breakpointToggled",
    "enabled",
    "foldToggled",
    "folded",
    "executionLineChanged",
    "codeFormatted",
    "snippetInserted",
    "snippet",
    "completionRequested",
    "visualizationRequested",
    "varName",
    "authorRegistered",
    "name",
    "color",
    "lineEditInfoChanged",
    "LineEditInfo",
    "info",
    "updateLineNumberAreaWidth",
    "newBlockCount",
    "highlightCurrentLine",
    "updateLineNumberArea",
    "rect",
    "dy",
    "onCompletionItemActivated",
    "item",
    "onFoldTimerTimeout",
    "onTextChanged",
    "position",
    "charsRemoved",
    "charsAdded"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__CodeEditor[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      17,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      11,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,  116,    2, 0x06,    1 /* Public */,
       5,    1,  121,    2, 0x06,    4 /* Public */,
       7,    2,  124,    2, 0x06,    6 /* Public */,
       9,    2,  129,    2, 0x06,    9 /* Public */,
      11,    1,  134,    2, 0x06,   12 /* Public */,
      12,    0,  137,    2, 0x06,   14 /* Public */,
      13,    1,  138,    2, 0x06,   15 /* Public */,
      15,    0,  141,    2, 0x06,   17 /* Public */,
      16,    1,  142,    2, 0x06,   18 /* Public */,
      18,    2,  145,    2, 0x06,   20 /* Public */,
      21,    2,  150,    2, 0x06,   23 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      24,    1,  155,    2, 0x08,   26 /* Private */,
      26,    0,  158,    2, 0x08,   28 /* Private */,
      27,    2,  159,    2, 0x08,   29 /* Private */,
      30,    1,  164,    2, 0x08,   32 /* Private */,
      32,    0,  167,    2, 0x08,   34 /* Private */,
      33,    3,  168,    2, 0x08,   35 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    3,    8,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    3,   10,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   17,
    QMetaType::Void, QMetaType::QString, QMetaType::QColor,   19,   20,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 22,    3,   23,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,   25,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QRect, QMetaType::Int,   28,   29,
    QMetaType::Void, QMetaType::QString,   31,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int,   34,   35,   36,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::CodeEditor::staticMetaObject = { {
    QMetaObject::SuperData::link<QPlainTextEdit::staticMetaObject>(),
    qt_meta_stringdata_proxima__CodeEditor.offsetsAndSizes,
    qt_meta_data_proxima__CodeEditor,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__CodeEditor_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CodeEditor, std::true_type>,
        // method 'cursorPositionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'fileModified'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'breakpointToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'foldToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'executionLineChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'codeFormatted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'snippetInserted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'completionRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'visualizationRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'authorRegistered'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QColor &, std::false_type>,
        // method 'lineEditInfoChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const LineEditInfo &, std::false_type>,
        // method 'updateLineNumberAreaWidth'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'highlightCurrentLine'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateLineNumberArea'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QRect &, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onCompletionItemActivated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onFoldTimerTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void proxima::CodeEditor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CodeEditor *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->cursorPositionChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->fileModified((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->breakpointToggled((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 3: _t->foldToggled((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 4: _t->executionLineChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->codeFormatted(); break;
        case 6: _t->snippetInserted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->completionRequested(); break;
        case 8: _t->visualizationRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 9: _t->authorRegistered((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QColor>>(_a[2]))); break;
        case 10: _t->lineEditInfoChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<LineEditInfo>>(_a[2]))); break;
        case 11: _t->updateLineNumberAreaWidth((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 12: _t->highlightCurrentLine(); break;
        case 13: _t->updateLineNumberArea((*reinterpret_cast< std::add_pointer_t<QRect>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 14: _t->onCompletionItemActivated((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 15: _t->onFoldTimerTimeout(); break;
        case 16: _t->onTextChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CodeEditor::*)(int , int );
            if (_t _q_method = &CodeEditor::cursorPositionChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CodeEditor::*)(bool );
            if (_t _q_method = &CodeEditor::fileModified; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CodeEditor::*)(int , bool );
            if (_t _q_method = &CodeEditor::breakpointToggled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (CodeEditor::*)(int , bool );
            if (_t _q_method = &CodeEditor::foldToggled; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (CodeEditor::*)(int );
            if (_t _q_method = &CodeEditor::executionLineChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (CodeEditor::*)();
            if (_t _q_method = &CodeEditor::codeFormatted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (CodeEditor::*)(const QString & );
            if (_t _q_method = &CodeEditor::snippetInserted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (CodeEditor::*)();
            if (_t _q_method = &CodeEditor::completionRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (CodeEditor::*)(const QString & );
            if (_t _q_method = &CodeEditor::visualizationRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (CodeEditor::*)(const QString & , const QColor & );
            if (_t _q_method = &CodeEditor::authorRegistered; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (CodeEditor::*)(int , const LineEditInfo & );
            if (_t _q_method = &CodeEditor::lineEditInfoChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
    }
}

const QMetaObject *proxima::CodeEditor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::CodeEditor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__CodeEditor.stringdata0))
        return static_cast<void*>(this);
    return QPlainTextEdit::qt_metacast(_clname);
}

int proxima::CodeEditor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPlainTextEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 17)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 17;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 17)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 17;
    }
    return _id;
}

// SIGNAL 0
void proxima::CodeEditor::cursorPositionChanged(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void proxima::CodeEditor::fileModified(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void proxima::CodeEditor::breakpointToggled(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void proxima::CodeEditor::foldToggled(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void proxima::CodeEditor::executionLineChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void proxima::CodeEditor::codeFormatted()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void proxima::CodeEditor::snippetInserted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void proxima::CodeEditor::completionRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void proxima::CodeEditor::visualizationRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void proxima::CodeEditor::authorRegistered(const QString & _t1, const QColor & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void proxima::CodeEditor::lineEditInfoChanged(int _t1, const LineEditInfo & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
