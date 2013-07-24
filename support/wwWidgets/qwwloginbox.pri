WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwloginboxiface.cpp
  HEADERS += $$WD/plugin/qwwloginboxiface.h
} else {
  SOURCES += $$WD/widgets/qwwloginbox/qwwloginbox.cpp
  HEADERS += $$WD/widgets/qwwloginbox/qwwloginbox.h
  FORMS   += $$WD/widgets/qwwloginbox/loginbox.ui
}
INCLUDEPATH += $$WD/widgets/qwwloginbox
INSTALL_HEADERS += $$WD/widgets/qwwloginbox/qwwloginbox.h $$WD/widgets/qwwloginbox/QwwLoginBox