#-------------------------------------------------
#
# Project created by QtCreator 2020-04-06T14:49:16
#
#-------------------------------------------------



QT       += core gui widgets datavisualization opengl multimediawidgets
LIBS     += -lopengl32 -lglu32 -openmp
QMAKE_CXXFLAGS += -openmp
#相对路径和绝对路径
#./ D:.....libopengl32.lib ./3rdparty/

#note: linux -fopenmp


TARGET = Recolor_GUI
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#INCLUDEPATH+=E:\Windows Kits\10\Include\10.0.20348.0\um
#INCLUDEPATH+=E:\Windows Kits\10\Include\10.0.19041.0\ucrt






CONFIG += c++11

SOURCES += \
    data.cpp \
        main.cpp \
        mainwindow.cpp \
    opengl3dwidget.cpp \
    openglwidget.cpp \
    paletteviewwidget.cpp \
    rgbwidget.cpp \
    utility.cpp \
    videowidget.cpp\
    imagewidget.cpp

HEADERS += \
    data.h \
        mainwindow.h \
    opengl3dwidget.h \
    openglwidget.h \
    paletteviewwidget.h \
    rgbwidget.h \
    utility.h \
    videowidget.h\
    imagewidget.h\
    algorithm/ModifiedKmeans.h\
    algorithm/PaletteBasedRecoloring.h\
    algorithm/SuperpixSeg.h\
    algorithm/Utility.h\


DEFINES += QUAZIP_BUILD

#INCLUDEPATH += $$PWDQt-Color-Widgets
include(Qt-Color-Widgets/color_widgets.pri)

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/lib/ -lopencv_world412
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/lib/ -lopencv_world412d

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/libopencv_world412.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/libopencv_world412d.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/lib/opencv_world412.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/lib/opencv_world412d.lib
