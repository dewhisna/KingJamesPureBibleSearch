!contains( included_modules, qtstyleplugins/src/qtstyleplugins.pri) {
		included_modules += qtstyleplugins/src/qtstyleplugins.pri

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

}
