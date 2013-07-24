WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwlistwidgetiface.cpp
  HEADERS += $$WD/plugin/qwwlistwidgetiface.h
} else {
  SOURCES += $$WD/widgets/qwwlistwidget/qwwlistwidget.cpp
  HEADERS += $$WD/widgets/qwwlistwidget/qwwlistwidget.h
}
INCLUDEPATH += $$WD/widgets/qwwlistwidget
INSTALL_HEADERS += $$WD/widgets/qwwlistwidget/qwwlistwidget.h $$WD/widgets/qwwlistwidget/QwwListWidget

