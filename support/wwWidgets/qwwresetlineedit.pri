WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwresetlineeditiface.cpp
  HEADERS += $$WD/plugin/qwwresetlineeditiface.h
} else {
  SOURCES += $$WD/widgets/qwwresetlineedit/qwwresetlineedit.cpp
  HEADERS += $$WD/widgets/qwwresetlineedit/qwwresetlineedit.h
}
INCLUDEPATH += $$WD/widgets/qwwresetlineedit
INSTALL_HEADERS += $$WD/widgets/qwwresetlineedit/qwwresetlineedit.h $$WD/widgets/qwwresetlineedit/QwwResetLineEdit