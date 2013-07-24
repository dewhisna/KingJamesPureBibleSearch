WD = $${PWD}
contains(TARGET, plugin) {
  SOURCES += $$WD/plugin/qwwbreadcrumbiface.cpp
  HEADERS += $$WD/plugin/qwwbreadcrumbiface.h
} else {
  SOURCES += $$WD/widgets/qwwbreadcrumb/qwwbreadcrumb.cpp
  HEADERS += $$WD/widgets/qwwbreadcrumb/qwwbreadcrumb.h
}
INCLUDEPATH += $$WD/widgets/qwwbreadcrumb
INSTALL_HEADERS += $$WD/widgets/qwwbreadcrumb/qwwbreadcrumb.h $$WD/widgets/qwwbredcrumb/QwwBreadCrumb

