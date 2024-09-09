/****************************************************************************
** Meta object code from reading C++ file 'SvgIcon.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../include/SvgIcon.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SvgIcon.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSSvgIconENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSSvgIconENDCLASS = QtMocHelpers::stringData(
    "SvgIcon",
    "color",
    "background",
    "opacity",
    "size",
    "scale",
    "border_color",
    "border_width"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSSvgIconENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       7,   14, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // properties: name, type, flags
       1, QMetaType::QColor, 0x00015103, uint(-1), 0,
       2, QMetaType::QColor, 0x00015103, uint(-1), 0,
       3, QMetaType::QReal, 0x00015103, uint(-1), 0,
       4, QMetaType::QSize, 0x00015103, uint(-1), 0,
       5, QMetaType::QReal, 0x00015103, uint(-1), 0,
       6, QMetaType::QColor, 0x00015003, uint(-1), 0,
       7, QMetaType::QReal, 0x00015003, uint(-1), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject SvgIcon::staticMetaObject = { {
    QMetaObject::SuperData::link<QSvgWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSSvgIconENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSSvgIconENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSSvgIconENDCLASS_t,
        // property 'color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'background'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'opacity'
        QtPrivate::TypeAndForceComplete<qreal, std::true_type>,
        // property 'size'
        QtPrivate::TypeAndForceComplete<QSize, std::true_type>,
        // property 'scale'
        QtPrivate::TypeAndForceComplete<qreal, std::true_type>,
        // property 'border_color'
        QtPrivate::TypeAndForceComplete<QColor, std::true_type>,
        // property 'border_width'
        QtPrivate::TypeAndForceComplete<qreal, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SvgIcon, std::true_type>
    >,
    nullptr
} };

void SvgIcon::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<SvgIcon *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QColor*>(_v) = _t->color(); break;
        case 1: *reinterpret_cast< QColor*>(_v) = _t->background(); break;
        case 2: *reinterpret_cast< qreal*>(_v) = _t->opacity(); break;
        case 3: *reinterpret_cast< QSize*>(_v) = _t->size(); break;
        case 4: *reinterpret_cast< qreal*>(_v) = _t->scale(); break;
        case 5: *reinterpret_cast< QColor*>(_v) = _t->borderColor(); break;
        case 6: *reinterpret_cast< qreal*>(_v) = _t->borderWidth(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<SvgIcon *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setColor(*reinterpret_cast< QColor*>(_v)); break;
        case 1: _t->setBackground(*reinterpret_cast< QColor*>(_v)); break;
        case 2: _t->setOpacity(*reinterpret_cast< qreal*>(_v)); break;
        case 3: _t->setSize(*reinterpret_cast< QSize*>(_v)); break;
        case 4: _t->setScale(*reinterpret_cast< qreal*>(_v)); break;
        case 5: _t->setBorderColor(*reinterpret_cast< QColor*>(_v)); break;
        case 6: _t->setBorderWidth(*reinterpret_cast< qreal*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *SvgIcon::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SvgIcon::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSSvgIconENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QSvgWidget::qt_metacast(_clname);
}

int SvgIcon::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QSvgWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
