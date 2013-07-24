WWWIDGETS = 	#ADDHERE#

wwwidgets_install_docs_html {
    INSTALL_DOCS += doc/html/*
    html_docs.files = $$INSTALL_DOCS
    html_docs.path = $$[QT_INSTALL_DOCS]/html/wwwidgets
    INSTALLS += html_docs
}

wwwidgets_install_docs_qch {
    INSTALL_QCH += doc/qch/wwwidgets.qch
    qch_docs.files = $$INSTALL_QCH
    qch_docs.path = $$[QT_INSTALL_DOCS]/qch
    INSTALLS += qch_docs
}

wwwidgets_register_qch {
    unix:register.commands = $$[QT_INSTALL_BINS]/assistant -register $$[QT_INSTALL_DOCS]/qch/wwwidgets.qch
    win32:register.commands = $$[QT_INSTALL_BINS]\assistant -register $$[QT_INSTALL_DOCS]\qch\wwwidgets.qch
    register.target = register
    QMAKE_EXTRA_TARGETS += register
}

wwwidgets_install_mkspecs {
    INSTALL_SPECS += wwwidgets.prf
    mkspecs.files = $$INSTALL_SPECS
    mkspecs.path = $$[QT_INSTALL_DATA]/mkspecs/features
    INSTALLS += mkspecs
}

unix {
    UI_DIR = .ui
    MOC_DIR = .moc
    RCC_DIR = .obj
    OBJECTS_DIR = .obj
    VERSION = 1.0.0
}

#win32 {
#    UI_DIR = ui
#    MOC_DIR = moc
#    RCC_DIR = obj
#    OBJECTS_DIR = obj
#}

include(widgets.pri)

buildhtmldocs.commands = cd qdoc && qdoc3 -creator wwwidgets-4.7.qdocconf
buildhtmldocs.target = htmldocs

buildhtmldocs46.commands = cd qdoc && qdoc3-4.6 wwwidgets-4.6.qdocconf
buildhtmldocs46.target = htmldocs-46


buildqchdocs46.commands = cd qdoc && qhelpgenerator html-wwwidgets-4.6/wwwidgets.qhp -o qch/wwwidgets-4.6.qch
buildqchdocs46.depends = htmldocs-46
buildqchdocs46.target = qdoc3-46

buildqchdocs.commands = cd qdoc && qhelpgenerator html-wwwidgets/wwwidgets.qhp -o qch/wwwidgets.qch
buildqchdocs.depends = htmldocs
buildqchdocs.target = qdoc3



QMAKE_EXTRA_TARGETS += buildqchdocs buildqchdocs46 buildhtmldocs buildhtmldocs46

