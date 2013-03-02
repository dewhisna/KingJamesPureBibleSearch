#-------------------------------------------------
#
# Project created by QtCreator 2012-09-22T22:59:45
#
#-------------------------------------------------

QT       += core gui sql

CONFIG += rtti

unix:!macx {
    CONFIG += static
    QMAKE_CXXFLAGS += -static
}

macx:CONFIG += x86 x86_64

#QTPLUGIN += qsqlite

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
    Highlighter.cpp \
    VerseListDelegate.cpp \
    KJVAboutDlg.cpp \
    SearchPhraseListModel.cpp \
    MimeHelper.cpp \
    ToolTipEdit.cpp \
    PersistentSettings.cpp \
    QtFindReplaceDialog/dialogs/findreplaceform.cpp \
    QtFindReplaceDialog/dialogs/findreplacedialog.cpp \
    QtFindReplaceDialog/dialogs/findform.cpp \
    QtFindReplaceDialog/dialogs/finddialog.cpp

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
    Highlighter.h \
    VerseListDelegate.h \
    KJVAboutDlg.h \
    version.h \
    SearchPhraseListModel.h \
    MimeHelper.h \
    ToolTipEdit.h \
    PersistentSettings.h \
    QtFindReplaceDialog/dialogs/findreplaceform.h \
    QtFindReplaceDialog/dialogs/findreplacedialog.h \
    QtFindReplaceDialog/dialogs/findform.h \
    QtFindReplaceDialog/dialogs/finddialog.h \
    QtFindReplaceDialog/dialogs/findreplace_global.h

FORMS    += KJVCanOpener.ui \
    KJVBrowser.ui \
    KJVSearchPhraseEdit.ui \
    KJVSearchCriteria.ui \
    KJVPassageNavigator.ui \
    KJVPassageNavigatorDlg.ui \
    KJVAboutDlg.ui \
    QtFindReplaceDialog/dialogs/findreplaceform.ui \
    QtFindReplaceDialog/dialogs/findreplacedialog.ui

RESOURCES += \
    KJVCanOpener.qrc


