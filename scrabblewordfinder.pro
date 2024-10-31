QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    scrabblewordfinder.cpp

HEADERS += \
    scrabblewordfinder.h

FORMS += \
    scrabblewordfinder.ui

DISTFILES += \
    wordinfo.db

RESOURCES += \
    scrabblewordfinder.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix{
    QMAKE_POST_LINK += $$quote(cp $$PWD/wordinfo.db $$OUT_PWD$$escape_expand(\n\t))
}
 win32 {
  QMAKE_POST_LINK += $$quote(cmd /c copy /y $$PWD\wordinfo.db $$OUT_PWD$$escape_expand(\n\t))
}

