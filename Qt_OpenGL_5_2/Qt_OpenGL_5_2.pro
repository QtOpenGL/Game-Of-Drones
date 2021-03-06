#-------------------------------------------------
#
# Project created by QtCreator 2017-03-27T22:45:41
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Qt_OpenGL_5_2
TEMPLATE = app

win32:RC_ICONS += Ikona.ico

include(Engine/Engine.pri)
include(Sound/Sound.pri)
include(Input/Input.pri)
include(Entity/Entity.pri)
include(Widgets/Widgets.pri)
include(Addons/Addons.pri)

SOURCES += main.cpp

HEADERS  +=  

RESOURCES += \
    resources.qrc

DISTFILES +=
