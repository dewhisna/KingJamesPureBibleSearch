#-------------------------------------------------
#
# Project created by QtCreator 2012-09-22T22:59:45
#
#-------------------------------------------------

QT       += core gui sql

CONFIG += rtti

# Miscellaneous Special-Testing Cache modes that can be enabled:
#DEFINES += VERSE_LIST_PLAIN_TEXT_CACHE
#DEFINES += VERSE_LIST_RICH_TEXT_CACHE
#DEFINES += BIBLE_DATABASE_RICH_TEXT_CACHE
#DEFINES += NORMALIZED_SEARCH_PHRASE_RESULTS_CACHE
declarative_debug:DEFINES += SIGNAL_SPY_DEBUG

unix:!macx {
	CONFIG += static
	QMAKE_CXXFLAGS += -static
}

macx:CONFIG += x86 x86_64
macx:static:LIBS += -lQtCore -lQtGui -lQtSql -dead_strip

# The following fixes a bad codegen, pointer diff to global weak in symbol vtable
#   error on Mac with the QtSharedPointer that we are using for the Bible Database.
#   It's only an issue when linking with the Static Qt we use for release:
macx:release:QMAKE_CXXFLAGS += -fvisibility=hidden -fvisibility-inlines-hidden

#QTPLUGIN += qsqlite

TARGET = KingJamesPureBibleSearch
TEMPLATE = app

RC_FILE += 	KJVCanOpener.rc  # descibes program icon and version

SOURCES += main.cpp \
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
	QtFindReplaceDialog/dialogs/finddialog.cpp \
	KJVSearchResult.cpp \
	KJVSearchSpec.cpp \
	ParseSymbols.cpp \
	VerseRichifier.cpp \
	DelayedExecutionTimer.cpp \
	ModelRowForwardIterator.cpp \
	ReflowDelegate.cpp \
	signalspy/Q4puGenericSignalSpy.cpp

HEADERS  += main.h \
	KJVCanOpener.h \
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
	QtFindReplaceDialog/dialogs/findreplace_global.h \
	KJVSearchResult.h \
	KJVSearchSpec.h \
	ParseSymbols.h \
	VerseRichifier.h \
	DelayedExecutionTimer.h \
	BusyCursor.h \
	ModelRowForwardIterator.h \
	ReflowDelegate.h \
	signalspy/Q4puGenericSignalSpy.h

FORMS    += KJVCanOpener.ui \
	KJVBrowser.ui \
	KJVSearchPhraseEdit.ui \
	KJVSearchCriteria.ui \
	KJVPassageNavigator.ui \
	KJVPassageNavigatorDlg.ui \
	KJVAboutDlg.ui \
	QtFindReplaceDialog/dialogs/findreplaceform.ui \
	QtFindReplaceDialog/dialogs/findreplacedialog.ui \
	KJVSearchSpec.ui

RESOURCES += \
	KJVCanOpener.qrc

# ICON for Mac OSX:
ICON = res/bible.icns

# Info.plist for Mac OSX:
# This is broken in qmake.  Copy KJVCanOpener.Info.plist.app to ~/Qt/.../mkspecs/default/Info.plist.app
#QMAKE_INFO_PLIST = KJVCanOpener.Info.plist.app

message($$CONFIG)
