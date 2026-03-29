/****************************************************************************
** Meta object code from reading C++ file 'FoldingSettingsDialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../centauri/src/ui/dialogs/FoldingSettingsDialog.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FoldingSettingsDialog.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_centauri__ui__FoldingSettingsDialog_t {
    uint offsetsAndSizes[20];
    char stringdata0[36];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[16];
    char stringdata4[15];
    char stringdata5[14];
    char stringdata6[14];
    char stringdata7[15];
    char stringdata8[17];
    char stringdata9[14];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_centauri__ui__FoldingSettingsDialog_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_centauri__ui__FoldingSettingsDialog_t qt_meta_stringdata_centauri__ui__FoldingSettingsDialog = {
    {
        QT_MOC_LITERAL(0, 35),  // "centauri::ui::FoldingSettings..."
        QT_MOC_LITERAL(36, 15),  // "settingsChanged"
        QT_MOC_LITERAL(52, 0),  // ""
        QT_MOC_LITERAL(53, 15),  // "settingsApplied"
        QT_MOC_LITERAL(69, 14),  // "onApplyClicked"
        QT_MOC_LITERAL(84, 13),  // "onSaveClicked"
        QT_MOC_LITERAL(98, 13),  // "onLoadClicked"
        QT_MOC_LITERAL(112, 14),  // "onResetClicked"
        QT_MOC_LITERAL(127, 16),  // "onPreviewChanged"
        QT_MOC_LITERAL(144, 13)   // "updatePreview"
    },
    "centauri::ui::FoldingSettingsDialog",
    "settingsChanged",
    "",
    "settingsApplied",
    "onApplyClicked",
    "onSaveClicked",
    "onLoadClicked",
    "onResetClicked",
    "onPreviewChanged",
    "updatePreview"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_centauri__ui__FoldingSettingsDialog[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   62,    2, 0x06,    1 /* Public */,
       3,    0,   63,    2, 0x06,    2 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       4,    0,   64,    2, 0x08,    3 /* Private */,
       5,    0,   65,    2, 0x08,    4 /* Private */,
       6,    0,   66,    2, 0x08,    5 /* Private */,
       7,    0,   67,    2, 0x08,    6 /* Private */,
       8,    0,   68,    2, 0x08,    7 /* Private */,
       9,    0,   69,    2, 0x08,    8 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject centauri::ui::FoldingSettingsDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_centauri__ui__FoldingSettingsDialog.offsetsAndSizes,
    qt_meta_data_centauri__ui__FoldingSettingsDialog,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_centauri__ui__FoldingSettingsDialog_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<FoldingSettingsDialog, std::true_type>,
        // method 'settingsChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'settingsApplied'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onApplyClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSaveClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLoadClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onResetClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPreviewChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updatePreview'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void centauri::ui::FoldingSettingsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FoldingSettingsDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->settingsChanged(); break;
        case 1: _t->settingsApplied(); break;
        case 2: _t->onApplyClicked(); break;
        case 3: _t->onSaveClicked(); break;
        case 4: _t->onLoadClicked(); break;
        case 5: _t->onResetClicked(); break;
        case 6: _t->onPreviewChanged(); break;
        case 7: _t->updatePreview(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FoldingSettingsDialog::*)();
            if (_t _q_method = &FoldingSettingsDialog::settingsChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (FoldingSettingsDialog::*)();
            if (_t _q_method = &FoldingSettingsDialog::settingsApplied; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }
    (void)_a;
}

const QMetaObject *centauri::ui::FoldingSettingsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *centauri::ui::FoldingSettingsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_centauri__ui__FoldingSettingsDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int centauri::ui::FoldingSettingsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
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
void centauri::ui::FoldingSettingsDialog::settingsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void centauri::ui::FoldingSettingsDialog::settingsApplied()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
