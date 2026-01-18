/****************************************************************************
** Meta object code from reading C++ file 'MapScreen.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/MapScreen.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MapScreen.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.1. It"
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
struct qt_meta_tag_ZN9MapScreenE_t {};
} // unnamed namespace

template <> constexpr inline auto MapScreen::qt_create_metaobjectdata<qt_meta_tag_ZN9MapScreenE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "MapScreen",
        "backToMenu",
        "",
        "levelSelected",
        "level",
        "combatLevelSelected",
        "shopEntered",
        "tavernEntered",
        "randomEventTriggered",
        "treasureFound",
        "bossBattleStarted",
        "resetMap"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'backToMenu'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'levelSelected'
        QtMocHelpers::SignalData<void(int)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Signal 'combatLevelSelected'
        QtMocHelpers::SignalData<void(int)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Signal 'shopEntered'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'tavernEntered'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'randomEventTriggered'
        QtMocHelpers::SignalData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'treasureFound'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'bossBattleStarted'
        QtMocHelpers::SignalData<void(int)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 4 },
        }}),
        // Slot 'resetMap'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<MapScreen, qt_meta_tag_ZN9MapScreenE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject MapScreen::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9MapScreenE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9MapScreenE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN9MapScreenE_t>.metaTypes,
    nullptr
} };

void MapScreen::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<MapScreen *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->backToMenu(); break;
        case 1: _t->levelSelected((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->combatLevelSelected((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->shopEntered(); break;
        case 4: _t->tavernEntered(); break;
        case 5: _t->randomEventTriggered(); break;
        case 6: _t->treasureFound(); break;
        case 7: _t->bossBattleStarted((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->resetMap(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (MapScreen::*)()>(_a, &MapScreen::backToMenu, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (MapScreen::*)(int )>(_a, &MapScreen::levelSelected, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (MapScreen::*)(int )>(_a, &MapScreen::combatLevelSelected, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (MapScreen::*)()>(_a, &MapScreen::shopEntered, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (MapScreen::*)()>(_a, &MapScreen::tavernEntered, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (MapScreen::*)()>(_a, &MapScreen::randomEventTriggered, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (MapScreen::*)()>(_a, &MapScreen::treasureFound, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (MapScreen::*)(int )>(_a, &MapScreen::bossBattleStarted, 7))
            return;
    }
}

const QMetaObject *MapScreen::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MapScreen::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN9MapScreenE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MapScreen::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void MapScreen::backToMenu()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MapScreen::levelSelected(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void MapScreen::combatLevelSelected(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void MapScreen::shopEntered()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void MapScreen::tavernEntered()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void MapScreen::randomEventTriggered()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void MapScreen::treasureFound()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void MapScreen::bossBattleStarted(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 7, nullptr, _t1);
}
QT_WARNING_POP
