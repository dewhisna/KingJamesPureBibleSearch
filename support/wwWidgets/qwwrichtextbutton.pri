WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwrichtextbuttoniface.cpp
  HEADERS += $$WD/plugin/qwwrichtextbuttoniface.h
} else {
  SOURCES += $$WD/widgets/qwwrichtextbutton/qwwrichtextbutton.cpp
  HEADERS += $$WD/widgets/qwwrichtextbutton/qwwrichtextbutton.h
}
INCLUDEPATH += $$WD/widgets/qwwrichtextbutton
INSTALL_HEADERS += $$WD/widgets/qwwrichtextbutton/qwwrichtextbutton.h $$WD/widgets/qwwrichtextbutton/QwwRichTextButton