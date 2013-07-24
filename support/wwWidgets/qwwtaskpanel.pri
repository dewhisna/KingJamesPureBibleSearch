WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwtaskpaneliface.cpp
  HEADERS += $$WD/plugin/qwwtaskpaneliface.h
} else {
  SOURCES += $$WD/widgets/qwwtaskpanel/qwwtaskpanel.cpp
  HEADERS += $$WD/widgets/qwwtaskpanel/qwwtaskpanel.h
  HEADERS += $$WD/widgets/qwwtaskpanel/qwwtaskpanel_p.h
}
INCLUDEPATH += $$WD/widgets/qwwtaskpanel
INSTALL_HEADERS += $$WD/widgets/qwwtaskpanel/qwwtaskpanel.h $$WD/widgets/qwwtaskpanel/QwwTaskPanel