WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwtextspinboxiface.cpp
  HEADERS += $$WD/plugin/qwwtextspinboxiface.h
} else {
  SOURCES += $$WD/widgets/qwwtextspinbox/qwwtextspinbox.cpp
  HEADERS += $$WD/widgets/qwwtextspinbox/qwwtextspinbox.h
}
INCLUDEPATH += $$WD/widgets/qwwtextspinbox
INSTALL_HEADERS += $$WD/widgets/qwwtextspinbox/qwwtextspinbox.h $$WD/widgets/qwwtextspinbox/QwwTextSpinBox

