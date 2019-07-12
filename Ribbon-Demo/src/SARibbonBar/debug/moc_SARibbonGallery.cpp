/****************************************************************************
** Meta object code from reading C++ file 'SARibbonGallery.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../SARibbonGallery.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SARibbonGallery.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SARibbonGallery_t {
    QByteArrayData data[7];
    char stringdata0[74];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SARibbonGallery_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SARibbonGallery_t qt_meta_stringdata_SARibbonGallery = {
    {
QT_MOC_LITERAL(0, 0, 15), // "SARibbonGallery"
QT_MOC_LITERAL(1, 16, 10), // "onPageDown"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 8), // "onPageUp"
QT_MOC_LITERAL(4, 37, 16), // "onShowMoreDetail"
QT_MOC_LITERAL(5, 54, 13), // "onItemClicked"
QT_MOC_LITERAL(6, 68, 5) // "index"

    },
    "SARibbonGallery\0onPageDown\0\0onPageUp\0"
    "onShowMoreDetail\0onItemClicked\0index"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SARibbonGallery[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x09 /* Protected */,
       3,    0,   35,    2, 0x09 /* Protected */,
       4,    0,   36,    2, 0x09 /* Protected */,
       5,    1,   37,    2, 0x09 /* Protected */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QModelIndex,    6,

       0        // eod
};

void SARibbonGallery::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SARibbonGallery *_t = static_cast<SARibbonGallery *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onPageDown(); break;
        case 1: _t->onPageUp(); break;
        case 2: _t->onShowMoreDetail(); break;
        case 3: _t->onItemClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject SARibbonGallery::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_SARibbonGallery.data,
      qt_meta_data_SARibbonGallery,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SARibbonGallery::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SARibbonGallery::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SARibbonGallery.stringdata0))
        return static_cast<void*>(this);
    return QFrame::qt_metacast(_clname);
}

int SARibbonGallery::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
struct qt_meta_stringdata_RibbonGalleryViewport_t {
    QByteArrayData data[1];
    char stringdata0[22];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_RibbonGalleryViewport_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_RibbonGalleryViewport_t qt_meta_stringdata_RibbonGalleryViewport = {
    {
QT_MOC_LITERAL(0, 0, 21) // "RibbonGalleryViewport"

    },
    "RibbonGalleryViewport"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RibbonGalleryViewport[] = {

 // content:
       7,       // revision
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

void RibbonGalleryViewport::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObject RibbonGalleryViewport::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_RibbonGalleryViewport.data,
      qt_meta_data_RibbonGalleryViewport,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *RibbonGalleryViewport::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RibbonGalleryViewport::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_RibbonGalleryViewport.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int RibbonGalleryViewport::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
