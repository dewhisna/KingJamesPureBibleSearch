#-------------------------------------------------
#
# Project created by QtCreator 2012-09-22T22:59:45
#
#-------------------------------------------------

QT       += core gui sql

TARGET = KJVCanOpener
TEMPLATE = app

RC_FILE += 	KJVCanOpener.rc  # descibes program icon and version

SOURCES += main.cpp\
	KJVCanOpener.cpp \
    CSV.cpp \
    dbstruct.cpp \
    BuildDB.cpp \
    ReadDB.cpp \
    KJVBrowser.cpp \
    KJVSearchPhraseEdit.cpp \
    VerseListModel.cpp \
    KJVSearchCriteria.cpp \
    PhraseListModel.cpp \
    KJVPassageNavigator.cpp \
    KJVPassageNavigatorDlg.cpp \
    PhraseEdit.cpp \
    ScriptureEdit.cpp \
    Highlighter.cpp

HEADERS  += KJVCanOpener.h \
    CSV.h \
    dbstruct.h \
    ReadDB.h \
    BuildDB.h \
    KJVBrowser.h \
    KJVSearchPhraseEdit.h \
    VerseListModel.h \
    KJVSearchCriteria.h \
    PhraseListModel.h \
    KJVPassageNavigator.h \
    KJVPassageNavigatorDlg.h \
    PhraseEdit.h \
    ScriptureEdit.h \
    Highlighter.h

FORMS    += KJVCanOpener.ui \
    KJVBrowser.ui \
    KJVSearchPhraseEdit.ui \
    KJVSearchCriteria.ui \
    KJVPassageNavigator.ui \
    KJVPassageNavigatorDlg.ui

RESOURCES += \
    KJVCanOpener.qrc


