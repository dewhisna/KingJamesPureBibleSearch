win32 {
    QMAKE_INSTALL_FILE = $(COPY)
    QMAKE_INSTALL_PROGRAM = $(COPY_FILE)
    QMAKE_INSTALL_DIR = $(COPY_DIR)
}


TEMPLATE = subdirs
SUBDIRS += widgets plugin

#SUBDIRS += examples

DISTFILES += wwwidgets_utils.pri logo.png wwWidgets.prf

CONFIG += ordered
CONFIG += wwwidgets_install_mkspecs \
          wwwidgets_install_docs_html wwwidgets_install_win32

NOTQCH = 1 2 3
!contains(NOTQCH, $$QT_MINOR_VERSION):CONFIG+=wwwidgets_install_docs_qch wwwidgets_register_qch

include(wwwidgets_utils.pri)

INSTALL_HEADERS += widgets/wwglobal.h
flat_headers.files = $$INSTALL_HEADERS
flat_headers.path = $$[QT_INSTALL_HEADERS]/wwWidgets
INSTALLS += flat_headers

