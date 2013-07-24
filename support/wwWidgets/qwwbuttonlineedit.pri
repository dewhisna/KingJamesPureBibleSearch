WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwbuttonlineeditiface.cpp
  HEADERS += $$WD/plugin/qwwbuttonlineeditiface.h
} else {
  SOURCES += $$WD/widgets/qwwbuttonlineedit/qwwbuttonlineedit.cpp
  HEADERS += $$WD/widgets/qwwbuttonlineedit/qwwbuttonlineedit.h
  HEADERS += $$WD/widgets/qwwbuttonlineedit/qwwbuttonlineedit_p.h
}
INCLUDEPATH += $$WD/widgets/qwwbuttonlineedit
INSTALL_HEADERS += $$WD/widgets/qwwbuttonlineedit/qwwbuttonlineedit.h $$WD/widgets/qwwbuttonlineedit/QwwButtonLineEdit
