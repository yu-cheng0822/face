QT += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android:  target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += C:/opencv-mingw/OpenCV-MinGW-Build-OpenCV-4.5.5-x64/include

LIBS += -LC:/opencv-mingw/OpenCV-MinGW-Build-OpenCV-4.5.5-x64/x64/mingw/lib \
        -lopencv_core455 \
        -lopencv_dnn455 \
        -lopencv_highgui455 \
        -lopencv_imgcodecs455 \
        -lopencv_imgproc455 \
        -lopencv_videoio455

LIBS += -LC:/opencv-mingw/OpenCV-MinGW-Build-OpenCV-4.5.5-x64/x64/mingw/bin
