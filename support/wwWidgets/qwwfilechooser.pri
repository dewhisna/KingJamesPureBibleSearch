WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwfilechooseriface.cpp
  HEADERS += $$WD/plugin/qwwfilechooseriface.h
} else {
  SOURCES += $$WD/widgets/qwwfilechooser/qwwfilechooser.cpp
  HEADERS += $$WD/widgets/qwwfilechooser/qwwfilechooser.h
}
INCLUDEPATH += $$WD/widgets/qwwfilechooser
INSTALL_HEADERS += $$WD/widgets/qwwfilechooser/qwwfilechooser.h $$WD/widgets/qwwfilechooser/QwwFileChooser