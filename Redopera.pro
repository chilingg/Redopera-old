TEMPLATE = app
CONFIG += c++11
CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt

DEPENDENT_PATH = $$PWD/../bin
R_DEPENDENT_PATH = $$PWD/../Redopera/Redlib

win32{
LIBS += -L$$DEPENDENT_PATH/libs/ -lglfw3 \
    -L$$DEPENDENT_PATH/libs/ -lfreetype

INCLUDEPATH += $$DEPENDENT_PATH/include
DEPENDPATH += $$DEPENDENT_PATH/include
}

INCLUDEPATH += $$R_DEPENDENT_PATH
DEPENDPATH += $$R_DEPENDENT_PATH

unix{
unix:!macx:
LIBS += -lglfw \
    -ldl \
    -lfreetype \

INCLUDEPATH += /usr/include/freetype2/
}

SOURCES += \
        Redlib/RController.cpp \
        Redlib/RDebug.cpp \
    Redlib/RFont.cpp \
    Redlib/RFontTexture.cpp \
        Redlib/RImage.cpp \
        Redlib/RObject.cpp \
        Redlib/RResource.cpp \
        Redlib/RShader.cpp \
        Redlib/RShaderProgram.cpp \
    Redlib/RSize.cpp \
    Redlib/RSprite.cpp \
    Redlib/RTexObject.cpp \
    Redlib/RTextline.cpp \
        Redlib/RTexture.cpp \
        Redlib/RWindow.cpp \
        Redlib/glad.c \
    Redlib/stb_image_implementatio.cpp \
    character.cpp \
        main.cpp \
    testclass.cpp \
    testctrl.cpp

HEADERS += \
    Redlib/RController.h \
    Redlib/RDebug.h \
    Redlib/RFont.h \
    Redlib/RFontTexture.h \
    Redlib/RImage.h \
    Redlib/RJoystick.h \
    Redlib/RJoystickEvent.h \
    Redlib/RKeyEvent.h \
    Redlib/RMouseEvent.h \
    Redlib/RObject.h \
    Redlib/RResizeEvent.h \
    Redlib/RResource.h \
    Redlib/RShader.h \
    Redlib/RShaderProgram.h \
    Redlib/RSize.h \
    Redlib/RSprite.h \
    Redlib/RTexObject.h \
    Redlib/RTextline.h \
    Redlib/RTexture.h \
    Redlib/RTimer.h \
    Redlib/RVolume.h \
    Redlib/RWheelEvent.h \
    Redlib/RWindow.h \
    Redlib/stb_image.h \
    character.h \
    constant.h \
    testclass.h \
    testctrl.h

DISTFILES += \
    shaders/fragment.frag \
    shaders/text.frag \
    shaders/texture.frag \
    shaders/vertex.vert
