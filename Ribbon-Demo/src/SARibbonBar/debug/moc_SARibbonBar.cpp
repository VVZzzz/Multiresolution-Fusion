/****************************************************************************
** Meta object code from reading C++ file 'SARibbonBar.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../SARibbonBar.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SARibbonBar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SARibbonBar_t {
    QByteArrayData data[17];
    char stringdata0[305];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SARibbonBar_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SARibbonBar_t qt_meta_stringdata_SARibbonBar = {
    {
QT_MOC_LITERAL(0, 0, 11), // "SARibbonBar"
QT_MOC_LITERAL(1, 12, 22), // "applitionButtonClicked"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 23), // "currentRibbonTabChanged"
QT_MOC_LITERAL(4, 60, 5), // "index"
QT_MOC_LITERAL(5, 66, 20), // "onWindowTitleChanged"
QT_MOC_LITERAL(6, 87, 5), // "title"
QT_MOC_LITERAL(7, 93, 19), // "onWindowIconChanged"
QT_MOC_LITERAL(8, 113, 4), // "icon"
QT_MOC_LITERAL(9, 118, 28), // "onCategoryWindowTitleChanged"
QT_MOC_LITERAL(10, 147, 18), // "onStackWidgetHided"
QT_MOC_LITERAL(11, 166, 25), // "onCurrentRibbonTabChanged"
QT_MOC_LITERAL(12, 192, 25), // "onCurrentRibbonTabClicked"
QT_MOC_LITERAL(13, 218, 31), // "onCurrentRibbonTabDoubleClicked"
QT_MOC_LITERAL(14, 250, 27), // "onContextsCategoryPageAdded"
QT_MOC_LITERAL(15, 278, 17), // "SARibbonCategory*"
QT_MOC_LITERAL(16, 296, 8) // "category"

    },
    "SARibbonBar\0applitionButtonClicked\0\0"
    "currentRibbonTabChanged\0index\0"
    "onWindowTitleChanged\0title\0"
    "onWindowIconChanged\0icon\0"
    "onCategoryWindowTitleChanged\0"
    "onStackWidgetHided\0onCurrentRibbonTabChanged\0"
    "onCurrentRibbonTabClicked\0"
    "onCurrentRibbonTabDoubleClicked\0"
    "onContextsCategoryPageAdded\0"
    "SARibbonCategory*\0category"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SARibbonBar[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x06 /* Public */,
       3,    1,   65,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   68,    2, 0x09 /* Protected */,
       7,    1,   71,    2, 0x09 /* Protected */,
       9,    1,   74,    2, 0x09 /* Protected */,
      10,    0,   77,    2, 0x09 /* Protected */,
      11,    1,   78,    2, 0x09 /* Protected */,
      12,    1,   81,    2, 0x09 /* Protected */,
      13,    1,   84,    2, 0x09 /* Protected */,
      14,    1,   87,    2, 0x09 /* Protected */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QIcon,    8,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, 0x80000000 | 15,   16,

       0        // eod
};

void SARibbonBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SARibbonBar *_t = static_cast<SARibbonBar *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->applitionButtonClicked(); break;
        case 1: _t->currentRibbonTabChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->onWindowTitleChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->onWindowIconChanged((*reinterpret_cast< const QIcon(*)>(_a[1]))); break;
        case 4: _t->onCategoryWindowTitleChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->onStackWidgetHided(); break;
        case 6: _t->onCurrentRibbonTabChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->onCurrentRibbonTabClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->onCurrentRibbonTabDoubleClicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->onContextsCategoryPageAdded((*reinterpret_cast< SARibbonCategory*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 9:
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
            typedef void (SARibbonBar::*_t)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SARibbonBar::applitionButtonClicked)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (SARibbonBar::*_t)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SARibbonBar::currentRibbonTabChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject SARibbonBar::staticMetaObject = {
    { &QMenuBar::staticMetaObject, qt_meta_stringdata_SARibbonBar.data,
      qt_meta_data_SARibbonBar,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SARibbonBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SARibbonBar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SARibbonBar.stringdata0))
        return static_cast<void*>(this);
    return QMenuBar::qt_metacast(_clname);
}

int SARibbonBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMenuBar::qt_metacall(_c, _id, _a);
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
void SARibbonBar::applitionButtonClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SARibbonBar::currentRibbonTabChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
