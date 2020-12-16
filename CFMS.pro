QT     += core gui sql charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogabout.cpp \
    dialoghistory.cpp \
    dialoglogin.cpp \
    dialogreg.cpp \
    dialogusermanage.cpp \
    main.cpp \
    mainwindow.cpp \
    qaesencryption.cpp \
    settings.cpp

HEADERS += \
    aesni/aesni-enc-cbc.h \
    aesni/aesni-enc-ecb.h \
    aesni/aesni-key-exp.h \
    dialogabout.h \
    dialoghistory.h \
    dialoglogin.h \
    dialogreg.h \
    dialogusermanage.h \
    mainwindow.h \
    qaesencryption.h \
    settings.h \
    sql.h

FORMS += \
    dialogabout.ui \
    dialoghistory.ui \
    dialoglogin.ui \
    dialogreg.ui \
    dialogusermanage.ui \
    mainwindow.ui

TRANSLATIONS += \
    CFMS_en_US.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

RC_FILE = exe_ico.rc

