WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwtipwidgetiface.cpp
  HEADERS += $$WD/plugin/qwwtipwidgetiface.h
} else {
  SOURCES += $$WD/widgets/qwwtipwidget/qwwtipwidget.cpp
  HEADERS += $$WD/widgets/qwwtipwidget/qwwtipwidget.h
}
INCLUDEPATH += $$WD/widgets/qwwtipwidget
INSTALL_HEADERS += $$WD/widgets/qwwtipwidget/qwwtipwidget.h $$WD/widgets/qwwtipwidget/QwwTipWidget
