!contains( included_modules, grantlee/textdocument/textdocument.pri) {
		included_modules += grantlee/textdocument/textdocument.pri

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS +=	\
		$$PWD/abstractmarkupbuilder.h \
		$$PWD/bbcodebuilder.h \
		$$PWD/grantlee_textdocument.h \
		$$PWD/markupdirector.h \
		$$PWD/markupdirector_p.h \
		$$PWD/mediawikimarkupbuilder.h \
		$$PWD/plaintextmarkupbuilder.h \
		$$PWD/texthtmlbuilder.h \
		$$PWD/grantlee_gui_export.h

SOURCES +=	\
		$$PWD/bbcodebuilder.cpp \
		$$PWD/markupdirector.cpp \
		$$PWD/mediawikimarkupbuilder.cpp \
		$$PWD/plaintextmarkupbuilder.cpp \
		$$PWD/texthtmlbuilder.cpp

#RESOURCES += 

}
