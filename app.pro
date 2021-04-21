#-------------------------------------------------
#
# Project created by QtCreator 2020-09-24T02:15:59
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = app
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


SOURCES += \
    chatbuf.pb.cc \
    consumer.cpp \
    consumer_manager.cpp \
    main.cpp \
    message_parser.cpp \
    messager.cpp \
    ndn_app.cpp \
    nfd_checker.cpp \
    nfd_report.cpp \
    nfd_status_http.cpp\
    repo-ng/repo-command-parameter.cpp \
    repo-ng/repo-command-response.cpp \
    repo_putfile.cpp \
    repo_getfile.cpp \
    repo_deletefile.cpp

HEADERS += \
    repo-ng/repo-command-parameter.hpp \
    repo-ng/repo-command-response.hpp \
    repo-ng/repo-tlv.hpp \
    chatbuf.pb.h \
    common.h \
    consumer_manager.h \
    message_parser.h \
    messager.h \
    ndn_app.h \
    nfd_checker.h \
    nfd_report.h \
    nfd_status_http.h \
    repo_putfile.h \
    repo_getfile.h \
    repo_deletefile.h


FORMS += \
        ndn_app.ui \
    consumer_manager.ui \
    messager.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libndn-cxx


SUBDIRS += \
    app.pro \



INCLUDEPATH += $$PWD/../../../../usr/local/include
DEPENDPATH += $$PWD/../../../../usr/local/include

unix|win32: LIBS += -lprotobuf
