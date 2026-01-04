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
# Cross-platform OpenCV configuration
unix {
    # Linux/Unix: Try to use pkg-config first
    CONFIG += link_pkgconfig
    PKGCONFIG += opencv4

    # Fallback to manual configuration if pkg-config fails
    !packagesExist(opencv4) {
        INCLUDEPATH += /usr/local/include/opencv4
        LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui \
                -lopencv_imgcodecs -lopencv_videoio -lopencv_dnn
    }
}

# Windows OpenCV configuration
# IMPORTANT: Adjust these paths to match your OpenCV installation!
# Default assumes OpenCV 4.x is installed at C:/opencv
win32 {
    # You can customize these variables to match your installation
    isEmpty(OPENCV_DIR) {
        OPENCV_DIR = C:/opencv/build
    }
    isEmpty(OPENCV_VERSION) {
        OPENCV_VERSION = 4120
    }

    message("Using OpenCV directory: $$OPENCV_DIR")
    message("Using OpenCV version: $$OPENCV_VERSION")

    INCLUDEPATH += $$OPENCV_DIR/include

    # Detect compiler type
    win32-msvc* {
        message("Detected MSVC compiler")
        OPENCV_LIB_DIR = $$OPENCV_DIR/x64/vc16/lib
        CONFIG(debug, debug|release) {
            LIBS += -L$$OPENCV_LIB_DIR -lopencv_world$${OPENCV_VERSION}d
        } else {
            LIBS += -L$$OPENCV_LIB_DIR -lopencv_world$${OPENCV_VERSION}
        }
    } else:win32-g++ {
        message("Detected MinGW compiler")
        OPENCV_LIB_DIR = $$OPENCV_DIR/x64/mingw/lib
        CONFIG(debug, debug|release) {
            LIBS += -L$$OPENCV_LIB_DIR -lopencv_world$${OPENCV_VERSION}d
        } else {
            LIBS += -L$$OPENCV_LIB_DIR -lopencv_world$${OPENCV_VERSION}
        }
    } else {
        warning("Unknown Windows compiler. Trying MinGW paths.")
        OPENCV_LIB_DIR = $$OPENCV_DIR/x64/mingw/lib
        CONFIG(debug, debug|release) {
            LIBS += -L$$OPENCV_LIB_DIR -lopencv_world$${OPENCV_VERSION}d
        } else {
            LIBS += -L$$OPENCV_LIB_DIR -lopencv_world$${OPENCV_VERSION}
        }
    }

    message("OpenCV library directory: $$OPENCV_LIB_DIR")
}
