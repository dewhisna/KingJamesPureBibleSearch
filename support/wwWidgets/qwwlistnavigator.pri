WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwlistnavigatoriface.cpp
  HEADERS += $$WD/plugin/qwwlistnavigatoriface.h
} else {
  SOURCES += $$WD/widgets/qwwlistnavigator/qwwlistnavigator.cpp
  HEADERS += $$WD/widgets/qwwlistnavigator/qwwlistnavigator.h
}
INCLUDEPATH += $$WD/widgets/qwwlistnavigator
INSTALL_HEADERS += $$WD/widgets/qwwlistnavigator/qwwlistnavigator.h $$WD/widgets/qwwlistnavigator/QwwListNavigator

