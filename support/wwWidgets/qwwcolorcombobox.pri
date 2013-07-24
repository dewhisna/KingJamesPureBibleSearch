WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwcolorcomboboxiface.cpp
  HEADERS += $$WD/plugin/qwwcolorcomboboxiface.h
} else {
  SOURCES += $$WD/widgets/qwwcolorcombobox/qwwcolorcombobox.cpp
  HEADERS += $$WD/widgets/qwwcolorcombobox/qwwcolorcombobox.h
}
INCLUDEPATH += $$WD/widgets/qwwcolorcombobox
INSTALL_HEADERS += $$WD/widgets/qwwcolorcombobox/qwwcolorcombobox.h $$WD/widgets/qwwcolorcombobox/QwwColorComboBox