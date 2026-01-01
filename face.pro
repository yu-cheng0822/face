QT       += core gui
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
INCLUDEPATH += C:/opencv/build/include

CONFIG(debug, debug|release) {
    # Debug 時連 debug 版的 lib
    LIBS += -LC:/opencv/build/x64/vc16/lib \
            -lopencv_world4120d
} else {
    # Release 時連 release 版的 lib
    LIBS += -LC:/opencv/build/x64/vc16/lib \
            -lopencv_world4120
}
