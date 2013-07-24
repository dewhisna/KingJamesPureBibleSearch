WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwrichtexteditiface.cpp
  HEADERS += $$WD/plugin/qwwrichtexteditiface.h
} else {
  SOURCES += $$WD/widgets/qwwrichtextedit/qwwrichtextedit.cpp
  HEADERS += $$WD/widgets/qwwrichtextedit/qwwrichtextedit.h
}
INCLUDEPATH += $$WD/widgets/qwwrichtextedit
INSTALL_HEADERS += $$WD/widgets/qwwrichtextedit/qwwrichtextedit.h $$WD/widgets/qwwrichtextedit/QwwRichTextEdit