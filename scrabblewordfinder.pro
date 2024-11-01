QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql
CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    scrabblewordfinder.cpp

HEADERS += \
    scrabblewordfinder.h

FORMS += \
    scrabblewordfinder.ui

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml \
    android/assets/wordinfo.db

RESOURCES += \
    scrabblewordfinder.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:!android {
    QMAKE_POST_LINK += $$quote(cp $$PWD/android/assets/wordinfo.db $$OUT_PWD$$escape_expand(\n\t))
}

win32 {
    PWD_WIN = $${PWD}
    OUT_PWD_WIN = $${OUT_PWD}
    PWD_WIN ~= s,/,\\,g # Windows Requires Extra Magic To Handle Paths
    OUT_PWD_WIN ~= s,/,\\,g # Windows Requires Extra Magic To Handle Paths

    QMAKE_POST_LINK += $$quote(cmd /c copy $${PWD_WIN}\\android\\assets\\wordinfo.db $${OUT_PWD_WIN}\\wordinfo.db)
}

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}
