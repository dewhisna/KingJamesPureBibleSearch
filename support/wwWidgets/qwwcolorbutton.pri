WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwcolorbuttoniface.cpp
  HEADERS += $$WD/plugin/qwwcolorbuttoniface.h
} else {
  SOURCES += $$WD/widgets/qwwcolorbutton/qwwcolorbutton.cpp
  HEADERS += $$WD/widgets/qwwcolorbutton/qwwcolorbutton.h
}
INCLUDEPATH += $$WD/widgets/qwwcolorbutton
INSTALL_HEADERS += $$WD/widgets/qwwcolorbutton/qwwcolorbutton.h $$WD/widgets/qwwcolorbutton/QwwColorButton