/****************************************************************************
** Meta object code from reading C++ file 'SARibbonContextCategory.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../SARibbonContextCategory.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SARibbonContextCategory.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SARibbonContextCategory_t {
    QByteArrayData data[5];
    char stringdata0[70];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SARibbonContextCategory_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SARibbonContextCategory_t qt_meta_stringdata_SARibbonContextCategory = {
    {
QT_MOC_LITERAL(0, 0, 23), // "SARibbonContextCategory"
QT_MOC_LITERAL(1, 24, 17), // "categoryPageAdded"
QT_MOC_LITERAL(2, 42, 0), // ""
QT_MOC_LITERAL(3, 43, 17), // "SARibbonCategory*"
QT_MOC_LITERAL(4, 61, 8) // "category"

    },
    "SARibbonContextCategory\0categoryPageAdded\0"
    "\0SARibbonCategory*\0category"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SARibbonContextCategory[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void SARibbonContextCategory::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SARibbonContextCategory *_t = static_cast<SARibbonContextCategory *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->categoryPageAdded((*reinterpret_cast< SARibbonCategory*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< SARibbonCategory* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            typedef void (SARibbonContextCategory::*_t)(SARibbonCategory * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SARibbonContextCategory::categoryPageAdded)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject SARibbonContextCategory::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SARibbonContextCategory.data,
      qt_meta_data_SARibbonContextCategory,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SARibbonContextCategory::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SARibbonContextCategory::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SARibbonContextCategory.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SARibbonContextCategory::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void SARibbonContextCategory::categoryPageAdded(SARibbonCategory * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
