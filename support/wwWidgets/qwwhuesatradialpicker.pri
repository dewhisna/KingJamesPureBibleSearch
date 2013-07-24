WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwhuesatradialpickeriface.cpp
  HEADERS += $$WD/plugin/qwwhuesatradialpickeriface.h
} else {
  SOURCES += $$WD/widgets/qwwhuesatradialpicker/qwwhuesatradialpicker.cpp
  HEADERS += $$WD/widgets/qwwhuesatradialpicker/qwwhuesatradialpicker.h
}
INCLUDEPATH += $$WD/widgets/qwwhuesatradialpicker
INSTALL_HEADERS += $$WD/widgets/qwwhuesatradialpicker/qwwhuesatradialpicker.h $$WD/widgets/qwwhuesatradialpicker/QwwHueSatRadialPicker