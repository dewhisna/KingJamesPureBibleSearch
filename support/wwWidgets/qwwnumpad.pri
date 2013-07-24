WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwnumpadiface.cpp
  HEADERS += $$WD/plugin/qwwnumpadiface.h
} else {
  SOURCES += $$WD/widgets/qwwnumpad/qwwnumpad.cpp
  HEADERS += $$WD/widgets/qwwnumpad/qwwnumpad.h
}
INCLUDEPATH += $$WD/widgets/qwwnumpad
INSTALL_HEADERS += $$WD/widgets/qwwnumpad/qwwnumpad.h $$WD/widgets/qwwnumpad/QwwNumPad