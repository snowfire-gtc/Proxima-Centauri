/****************************************************************************
** Meta object code from reading C++ file 'CompatibilityReportDialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/ui/dialogs/CompatibilityReportDialog.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CompatibilityReportDialog.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_centauri__ui__CompatibilityReportDialog_t {
    uint offsetsAndSizes[24];
    char stringdata0[40];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[17];
    char stringdata4[14];
    char stringdata5[5];
    char stringdata6[14];
    char stringdata7[17];
    char stringdata8[5];
    char stringdata9[16];
    char stringdata10[15];
    char stringdata11[15];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_centauri__ui__CompatibilityReportDialog_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_centauri__ui__CompatibilityReportDialog_t qt_meta_stringdata_centauri__ui__CompatibilityReportDialog = {
    {
        QT_MOC_LITERAL(0, 39),  // "centauri::ui::CompatibilityRe..."
        QT_MOC_LITERAL(40, 15),  // "analysisStarted"
        QT_MOC_LITERAL(56, 0),  // ""
        QT_MOC_LITERAL(57, 16),  // "analysisFinished"
        QT_MOC_LITERAL(74, 13),  // "issueSelected"
        QT_MOC_LITERAL(88, 4),  // "line"
        QT_MOC_LITERAL(93, 13),  // "onItemClicked"
        QT_MOC_LITERAL(107, 16),  // "QTreeWidgetItem*"
        QT_MOC_LITERAL(124, 4),  // "item"
        QT_MOC_LITERAL(129, 15),  // "onExportClicked"
        QT_MOC_LITERAL(145, 14),  // "onCloseClicked"
        QT_MOC_LITERAL(160, 14)   // "updateProgress"
    },
    "centauri::ui::CompatibilityReportDialog",
    "analysisStarted",
    "",
    "analysisFinished",
    "issueSelected",
    "line",
    "onItemClicked",
    "QTreeWidgetItem*",
    "item",
    "onExportClicked",
    "onCloseClicked",
    "updateProgress"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_centauri__ui__CompatibilityReportDialog[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   56,    2, 0x06,    1 /* Public */,
       3,    0,   57,    2, 0x06,    2 /* Public */,
       4,    1,   58,    2, 0x06,    3 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       6,    1,   61,    2, 0x08,    5 /* Private */,
       9,    0,   64,    2, 0x08,    7 /* Private */,
      10,    0,   65,    2, 0x08,    8 /* Private */,
      11,    0,   66,    2, 0x08,    9 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject centauri::ui::CompatibilityReportDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_centauri__ui__CompatibilityReportDialog.offsetsAndSizes,
    qt_meta_data_centauri__ui__CompatibilityReportDialog,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_centauri__ui__CompatibilityReportDialog_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CompatibilityReportDialog, std::true_type>,
        // method 'analysisStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'analysisFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'issueSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onItemClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QTreeWidgetItem *, std::false_type>,
        // method 'onExportClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCloseClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void centauri::ui::CompatibilityReportDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CompatibilityReportDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->analysisStarted(); break;
        case 1: _t->analysisFinished(); break;
        case 2: _t->issueSelected((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->onItemClicked((*reinterpret_cast< std::add_pointer_t<QTreeWidgetItem*>>(_a[1]))); break;
        case 4: _t->onExportClicked(); break;
        case 5: _t->onCloseClicked(); break;
        case 6: _t->updateProgress(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CompatibilityReportDialog::*)();
            if (_t _q_method = &CompatibilityReportDialog::analysisStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CompatibilityReportDialog::*)();
            if (_t _q_method = &CompatibilityReportDialog::analysisFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CompatibilityReportDialog::*)(int );
            if (_t _q_method = &CompatibilityReportDialog::issueSelected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *centauri::ui::CompatibilityReportDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *centauri::ui::CompatibilityReportDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_centauri__ui__CompatibilityReportDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int centauri::ui::CompatibilityReportDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void centauri::ui::CompatibilityReportDialog::analysisStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void centauri::ui::CompatibilityReportDialog::analysisFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void centauri::ui::CompatibilityReportDialog::issueSelected(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
