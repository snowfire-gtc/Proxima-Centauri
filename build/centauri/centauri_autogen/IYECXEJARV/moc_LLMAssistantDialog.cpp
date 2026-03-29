/****************************************************************************
** Meta object code from reading C++ file 'LLMAssistantDialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/ui/dialogs/LLMAssistantDialog.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LLMAssistantDialog.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_proxima__ModificationItemWidget_t {
    uint offsetsAndSizes[26];
    char stringdata0[32];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[8];
    char stringdata4[9];
    char stringdata5[22];
    char stringdata6[23];
    char stringdata7[18];
    char stringdata8[18];
    char stringdata9[8];
    char stringdata10[22];
    char stringdata11[23];
    char stringdata12[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__ModificationItemWidget_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__ModificationItemWidget_t qt_meta_stringdata_proxima__ModificationItemWidget = {
    {
        QT_MOC_LITERAL(0, 31),  // "proxima::ModificationItemWidget"
        QT_MOC_LITERAL(32, 17),  // "acceptanceChanged"
        QT_MOC_LITERAL(50, 0),  // ""
        QT_MOC_LITERAL(51, 7),  // "blockId"
        QT_MOC_LITERAL(59, 8),  // "accepted"
        QT_MOC_LITERAL(68, 21),  // "viewOriginalRequested"
        QT_MOC_LITERAL(90, 22),  // "viewSuggestedRequested"
        QT_MOC_LITERAL(113, 17),  // "showDiffRequested"
        QT_MOC_LITERAL(131, 17),  // "onCheckBoxToggled"
        QT_MOC_LITERAL(149, 7),  // "checked"
        QT_MOC_LITERAL(157, 21),  // "onViewOriginalClicked"
        QT_MOC_LITERAL(179, 22),  // "onViewSuggestedClicked"
        QT_MOC_LITERAL(202, 17)   // "onShowDiffClicked"
    },
    "proxima::ModificationItemWidget",
    "acceptanceChanged",
    "",
    "blockId",
    "accepted",
    "viewOriginalRequested",
    "viewSuggestedRequested",
    "showDiffRequested",
    "onCheckBoxToggled",
    "checked",
    "onViewOriginalClicked",
    "onViewSuggestedClicked",
    "onShowDiffClicked"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__ModificationItemWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   62,    2, 0x06,    1 /* Public */,
       5,    1,   67,    2, 0x06,    4 /* Public */,
       6,    1,   70,    2, 0x06,    6 /* Public */,
       7,    1,   73,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       8,    1,   76,    2, 0x08,   10 /* Private */,
      10,    0,   79,    2, 0x08,   12 /* Private */,
      11,    0,   80,    2, 0x08,   13 /* Private */,
      12,    0,   81,    2, 0x08,   14 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,    3,    4,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    9,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::ModificationItemWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_proxima__ModificationItemWidget.offsetsAndSizes,
    qt_meta_data_proxima__ModificationItemWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__ModificationItemWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ModificationItemWidget, std::true_type>,
        // method 'acceptanceChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'viewOriginalRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'viewSuggestedRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'showDiffRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onCheckBoxToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onViewOriginalClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onViewSuggestedClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onShowDiffClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void proxima::ModificationItemWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ModificationItemWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->acceptanceChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 1: _t->viewOriginalRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->viewSuggestedRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->showDiffRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 4: _t->onCheckBoxToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 5: _t->onViewOriginalClicked(); break;
        case 6: _t->onViewSuggestedClicked(); break;
        case 7: _t->onShowDiffClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ModificationItemWidget::*)(int , bool );
            if (_t _q_method = &ModificationItemWidget::acceptanceChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ModificationItemWidget::*)(int );
            if (_t _q_method = &ModificationItemWidget::viewOriginalRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ModificationItemWidget::*)(int );
            if (_t _q_method = &ModificationItemWidget::viewSuggestedRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ModificationItemWidget::*)(int );
            if (_t _q_method = &ModificationItemWidget::showDiffRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *proxima::ModificationItemWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::ModificationItemWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__ModificationItemWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int proxima::ModificationItemWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void proxima::ModificationItemWidget::acceptanceChanged(int _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void proxima::ModificationItemWidget::viewOriginalRequested(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void proxima::ModificationItemWidget::viewSuggestedRequested(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void proxima::ModificationItemWidget::showDiffRequested(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
namespace {
struct qt_meta_stringdata_proxima__LLMAssistantDialog_t {
    uint offsetsAndSizes[84];
    char stringdata0[28];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[16];
    char stringdata4[16];
    char stringdata5[11];
    char stringdata6[12];
    char stringdata7[12];
    char stringdata8[12];
    char stringdata9[14];
    char stringdata10[18];
    char stringdata11[18];
    char stringdata12[19];
    char stringdata13[14];
    char stringdata14[20];
    char stringdata15[20];
    char stringdata16[19];
    char stringdata17[16];
    char stringdata18[5];
    char stringdata19[24];
    char stringdata20[9];
    char stringdata21[26];
    char stringdata22[6];
    char stringdata23[23];
    char stringdata24[8];
    char stringdata25[9];
    char stringdata26[15];
    char stringdata27[16];
    char stringdata28[11];
    char stringdata29[14];
    char stringdata30[22];
    char stringdata31[12];
    char stringdata32[11];
    char stringdata33[6];
    char stringdata34[23];
    char stringdata35[24];
    char stringdata36[10];
    char stringdata37[18];
    char stringdata38[17];
    char stringdata39[19];
    char stringdata40[13];
    char stringdata41[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_proxima__LLMAssistantDialog_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_proxima__LLMAssistantDialog_t qt_meta_stringdata_proxima__LLMAssistantDialog = {
    {
        QT_MOC_LITERAL(0, 27),  // "proxima::LLMAssistantDialog"
        QT_MOC_LITERAL(28, 19),  // "onRequestAssistance"
        QT_MOC_LITERAL(48, 0),  // ""
        QT_MOC_LITERAL(49, 15),  // "onCancelRequest"
        QT_MOC_LITERAL(65, 15),  // "onApplySelected"
        QT_MOC_LITERAL(81, 10),  // "onApplyAll"
        QT_MOC_LITERAL(92, 11),  // "onRejectAll"
        QT_MOC_LITERAL(104, 11),  // "onAcceptAll"
        QT_MOC_LITERAL(116, 11),  // "onSelectAll"
        QT_MOC_LITERAL(128, 13),  // "onDeselectAll"
        QT_MOC_LITERAL(142, 17),  // "onInvertSelection"
        QT_MOC_LITERAL(160, 17),  // "onViewAllOriginal"
        QT_MOC_LITERAL(178, 18),  // "onViewAllSuggested"
        QT_MOC_LITERAL(197, 13),  // "onViewAllDiff"
        QT_MOC_LITERAL(211, 19),  // "onExportSuggestions"
        QT_MOC_LITERAL(231, 19),  // "onImportSuggestions"
        QT_MOC_LITERAL(251, 18),  // "onClearSuggestions"
        QT_MOC_LITERAL(270, 15),  // "onFilterChanged"
        QT_MOC_LITERAL(286, 4),  // "text"
        QT_MOC_LITERAL(291, 23),  // "onCategoryFilterChanged"
        QT_MOC_LITERAL(315, 8),  // "category"
        QT_MOC_LITERAL(324, 25),  // "onConfidenceFilterChanged"
        QT_MOC_LITERAL(350, 5),  // "value"
        QT_MOC_LITERAL(356, 22),  // "onModificationAccepted"
        QT_MOC_LITERAL(379, 7),  // "blockId"
        QT_MOC_LITERAL(387, 8),  // "accepted"
        QT_MOC_LITERAL(396, 14),  // "onViewOriginal"
        QT_MOC_LITERAL(411, 15),  // "onViewSuggested"
        QT_MOC_LITERAL(427, 10),  // "onShowDiff"
        QT_MOC_LITERAL(438, 13),  // "onLLMResponse"
        QT_MOC_LITERAL(452, 21),  // "QList<CodeSuggestion>"
        QT_MOC_LITERAL(474, 11),  // "suggestions"
        QT_MOC_LITERAL(486, 10),  // "onLLMError"
        QT_MOC_LITERAL(497, 5),  // "error"
        QT_MOC_LITERAL(503, 22),  // "onLLMProcessingStarted"
        QT_MOC_LITERAL(526, 23),  // "onLLMProcessingFinished"
        QT_MOC_LITERAL(550, 9),  // "onTimeout"
        QT_MOC_LITERAL(560, 17),  // "updateProgressBar"
        QT_MOC_LITERAL(578, 16),  // "updateStatistics"
        QT_MOC_LITERAL(595, 18),  // "updateButtonStates"
        QT_MOC_LITERAL(614, 12),  // "onTabChanged"
        QT_MOC_LITERAL(627, 5)   // "index"
    },
    "proxima::LLMAssistantDialog",
    "onRequestAssistance",
    "",
    "onCancelRequest",
    "onApplySelected",
    "onApplyAll",
    "onRejectAll",
    "onAcceptAll",
    "onSelectAll",
    "onDeselectAll",
    "onInvertSelection",
    "onViewAllOriginal",
    "onViewAllSuggested",
    "onViewAllDiff",
    "onExportSuggestions",
    "onImportSuggestions",
    "onClearSuggestions",
    "onFilterChanged",
    "text",
    "onCategoryFilterChanged",
    "category",
    "onConfidenceFilterChanged",
    "value",
    "onModificationAccepted",
    "blockId",
    "accepted",
    "onViewOriginal",
    "onViewSuggested",
    "onShowDiff",
    "onLLMResponse",
    "QList<CodeSuggestion>",
    "suggestions",
    "onLLMError",
    "error",
    "onLLMProcessingStarted",
    "onLLMProcessingFinished",
    "onTimeout",
    "updateProgressBar",
    "updateStatistics",
    "updateButtonStates",
    "onTabChanged",
    "index"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_proxima__LLMAssistantDialog[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      31,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  200,    2, 0x0a,    1 /* Public */,
       3,    0,  201,    2, 0x0a,    2 /* Public */,
       4,    0,  202,    2, 0x0a,    3 /* Public */,
       5,    0,  203,    2, 0x0a,    4 /* Public */,
       6,    0,  204,    2, 0x0a,    5 /* Public */,
       7,    0,  205,    2, 0x0a,    6 /* Public */,
       8,    0,  206,    2, 0x0a,    7 /* Public */,
       9,    0,  207,    2, 0x0a,    8 /* Public */,
      10,    0,  208,    2, 0x0a,    9 /* Public */,
      11,    0,  209,    2, 0x0a,   10 /* Public */,
      12,    0,  210,    2, 0x0a,   11 /* Public */,
      13,    0,  211,    2, 0x0a,   12 /* Public */,
      14,    0,  212,    2, 0x0a,   13 /* Public */,
      15,    0,  213,    2, 0x0a,   14 /* Public */,
      16,    0,  214,    2, 0x0a,   15 /* Public */,
      17,    1,  215,    2, 0x0a,   16 /* Public */,
      19,    1,  218,    2, 0x0a,   18 /* Public */,
      21,    1,  221,    2, 0x0a,   20 /* Public */,
      23,    2,  224,    2, 0x0a,   22 /* Public */,
      26,    1,  229,    2, 0x0a,   25 /* Public */,
      27,    1,  232,    2, 0x0a,   27 /* Public */,
      28,    1,  235,    2, 0x0a,   29 /* Public */,
      29,    1,  238,    2, 0x0a,   31 /* Public */,
      32,    1,  241,    2, 0x0a,   33 /* Public */,
      34,    0,  244,    2, 0x0a,   35 /* Public */,
      35,    0,  245,    2, 0x0a,   36 /* Public */,
      36,    0,  246,    2, 0x08,   37 /* Private */,
      37,    0,  247,    2, 0x08,   38 /* Private */,
      38,    0,  248,    2, 0x08,   39 /* Private */,
      39,    0,  249,    2, 0x08,   40 /* Private */,
      40,    1,  250,    2, 0x08,   41 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   18,
    QMetaType::Void, QMetaType::QString,   20,
    QMetaType::Void, QMetaType::Double,   22,
    QMetaType::Void, QMetaType::Int, QMetaType::Bool,   24,   25,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, QMetaType::Int,   24,
    QMetaType::Void, 0x80000000 | 30,   31,
    QMetaType::Void, QMetaType::QString,   33,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   41,

       0        // eod
};

Q_CONSTINIT const QMetaObject proxima::LLMAssistantDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_proxima__LLMAssistantDialog.offsetsAndSizes,
    qt_meta_data_proxima__LLMAssistantDialog,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_proxima__LLMAssistantDialog_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<LLMAssistantDialog, std::true_type>,
        // method 'onRequestAssistance'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCancelRequest'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onApplySelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onApplyAll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRejectAll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onAcceptAll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSelectAll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDeselectAll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onInvertSelection'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onViewAllOriginal'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onViewAllSuggested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onViewAllDiff'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onExportSuggestions'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onImportSuggestions'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onClearSuggestions'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFilterChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onCategoryFilterChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onConfidenceFilterChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'onModificationAccepted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onViewOriginal'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onViewSuggested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onShowDiff'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onLLMResponse'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QVector<CodeSuggestion> &, std::false_type>,
        // method 'onLLMError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onLLMProcessingStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLLMProcessingFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTimeout'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateProgressBar'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateStatistics'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateButtonStates'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTabChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void proxima::LLMAssistantDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<LLMAssistantDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onRequestAssistance(); break;
        case 1: _t->onCancelRequest(); break;
        case 2: _t->onApplySelected(); break;
        case 3: _t->onApplyAll(); break;
        case 4: _t->onRejectAll(); break;
        case 5: _t->onAcceptAll(); break;
        case 6: _t->onSelectAll(); break;
        case 7: _t->onDeselectAll(); break;
        case 8: _t->onInvertSelection(); break;
        case 9: _t->onViewAllOriginal(); break;
        case 10: _t->onViewAllSuggested(); break;
        case 11: _t->onViewAllDiff(); break;
        case 12: _t->onExportSuggestions(); break;
        case 13: _t->onImportSuggestions(); break;
        case 14: _t->onClearSuggestions(); break;
        case 15: _t->onFilterChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->onCategoryFilterChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 17: _t->onConfidenceFilterChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 18: _t->onModificationAccepted((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 19: _t->onViewOriginal((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 20: _t->onViewSuggested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 21: _t->onShowDiff((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 22: _t->onLLMResponse((*reinterpret_cast< std::add_pointer_t<QList<CodeSuggestion>>>(_a[1]))); break;
        case 23: _t->onLLMError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 24: _t->onLLMProcessingStarted(); break;
        case 25: _t->onLLMProcessingFinished(); break;
        case 26: _t->onTimeout(); break;
        case 27: _t->updateProgressBar(); break;
        case 28: _t->updateStatistics(); break;
        case 29: _t->updateButtonStates(); break;
        case 30: _t->onTabChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *proxima::LLMAssistantDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proxima::LLMAssistantDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proxima__LLMAssistantDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int proxima::LLMAssistantDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 31)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 31;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 31)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 31;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
